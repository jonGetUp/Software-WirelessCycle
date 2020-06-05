/* ************************************************************************************************
 * Copyright (c) 2019, HES-SO Valais-Wallis, HEI, Infotronics
 * All rights reserved.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
 * WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ************************************************************************************************** */
package ch.hevs.heatflow.ble.services.adopted;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.support.annotation.NonNull;
import android.util.Log;

import java.util.LinkedList;
import java.util.List;

import ch.hevs.ble.lib.core.BleService;
import ch.hevs.ble.lib.events.commands.ReadCharValue;
import ch.hevs.ble.lib.events.commands.WriteCharConfig;
import ch.hevs.ble.lib.events.responses.BleGattResponse;
import ch.hevs.ble.lib.events.responses.CharDescWritten;
import ch.hevs.ble.lib.events.responses.OnBleCharacteristicChanged;
import ch.hevs.ble.lib.events.responses.OnBleCharacteristicDescriptorChanged;
import ch.hevs.lib.soprod.watchkit.implementation.ble.services.DefaultService;

import static android.bluetooth.BluetoothGattCharacteristic.FORMAT_UINT8;

/**
 * For Internal Use only !
 * @author Patrice Rudaz (patrice.rudaz@hevs.ch)
 * @version 1.0 - 2017/06/15
 */
public class BatteryService extends DefaultService {

    private final static String TAG = "BatteryService";

    /** SNS Notification Settings */
    private BluetoothGattCharacteristic mBatteryLevelChar;
    private boolean mBatteryLevelRequested;
    private boolean mBatteryLevelNotificationRequested;

    /** Battery level in percent */
    private Integer mBatteryLevel = null;

    private boolean isBatteryLevelNotificationEnabled = false;


    /* ****************************************************************************************** */
    /*                                                                                            */
    /*                               CONSTRUCTOR'S SECTION                                        */
    /*                                                                                            */
    /* ****************************************************************************************** */
    /** constructor */
    public BatteryService(BleService bleLibService) {
        super(bleLibService);
        invalidateService();    // init
    }


    /* ****************************************************************************************** */
    /*                                                                                            */
    /*                                 LISTENER'S SECTION                                         */
    /*                                                                                            */
    /* ****************************************************************************************** */
    @NonNull
    private final List<BatteryServiceListener> mListeners = new LinkedList<>();

    /**
     * Register an observer. Do nothing if the observer is already registered.
     *
     * @param listener the observer to register has an observer
     */
    public void addListener(@NonNull BatteryServiceListener listener) {
        if (mListeners.contains(listener))
            return;

        mListeners.add(listener);
    }

    /**
     * Unregister the observer.
     *
     * @param listener the observer to unregister
     */
    public void removeListener(@NonNull BatteryServiceListener listener) {
        mListeners.remove(listener);
    }

    /**
     * Removed / unregister all the observers.
     */
    public void removeListeners() {
        mListeners.clear();
    }


    /* ****************************************************************************************** */
    /*                                                                                            */
    /*                                  GETTER'S SECTION                                          */
    /*                                                                                            */
    /* ****************************************************************************************** */


    /* ****************************************************************************************** */
    /*                                                                                            */
    /*                                  PUBLIC'S SECTION                                          */
    /*                                                                                            */
    /* ****************************************************************************************** */
    /**
     * Returns the Battery level expressed in percent.
     * <p>
     *     If the BLE characteristic was ner read, the method will ask for by sending a BLE read request.
     * </p>
     * <p>
     *     Important:<br/>
     *     <b>You should enable the notification {@link BatteryService#enableNotification(boolean)} on
     *     the BLE charcteristic to be notified when its level change.</b>
     * </p>
     *
     */
    public int getBatteryLevel() {
        if (mBatteryLevel == null) {
            if (readBatteryLevel())
                return 0;
            else
                return -1;
        } else {
            return mBatteryLevel;
        }
    }

    /**
     * Enable or disable the notification on the BLE characterisitc related to the battery's level.
     * <p>
     *     The notification let any listener be notified when the level of the battery changes.
     * </p>
     *
     * @param enable    To enable or disable the notification feature on the battery level's characteristic
     * @return
     */
    public boolean enableNotification(boolean enable) {
        if (mBatteryLevelChar != null) {
            if (!mBatteryLevelNotificationRequested) {
                mBatteryLevelNotificationRequested = true;
                if (enable)
                    mBleLib.queueCommand(WriteCharConfig.enableNotification(mBatteryLevelChar), () -> {
                        mBatteryLevelNotificationRequested = false;
                        return null;
                    });
                else
                    mBleLib.queueCommand(WriteCharConfig.disableNotification(mBatteryLevelChar), () -> {
                        mBatteryLevelNotificationRequested = false;
                        return null;
                    });
            }
            return mBatteryLevelNotificationRequested;
        } else {
            Log.w(TAG, "mBatteryLevelChar is NULL !");
        }
        return false;
    }

    /**
     * Set up the battery service and its battery level 's characteristic from the discovered service over the air.
     * @param service   The discovered service.
     */
    public void discoverService(@NonNull final BluetoothGattService service) {
        if (mService == null) {
            Log.i(TAG, "- BAS available !");

            // Assign service and extract the characteristic from the service
            mService          = service;
            mBatteryLevelChar = getCharacteristic(mService, BtSigUuid.CHAR_BATTERY_LEVEL);

            mBleLib.addOnBleCharacteristicChangedListener(mBatteryLevelChar, new OnBleCharacteristicChanged<BluetoothGattCharacteristic, Integer>() {
                @Override
                public Void call() {
                    decodeBatteryLevel(getCharacteristic(), getStatus());
                    return null;
                }
            });
            mBleLib.addOnCharacteristicDescriptorChangedListener(mBatteryLevelChar, new OnBleCharacteristicDescriptorChanged<CharDescWritten.CCCDValue, Integer>() {
                @Override
                public Void call() {
                    isBatteryLevelNotificationEnabled = isNotificationEnabled();
                    notifyListenerOnBatteryLevelNotificationChanged();
                    return null;
                }
            });

            resetParameter();
        }
    }


    /* ****************************************************************************************** */
    /*                                                                                            */
    /*                                 OVERRIDE'S SECTION                                         */
    /*                                                                                            */
    /* ****************************************************************************************** */
    /**
     * Invalidate the current service by setting all characteristics and service to `null`
     */
    @Override
    protected void invalidateService()  {
        super.invalidateService();

        // invalidate all characteristics
        mBatteryLevelChar = null;

        resetParameter();
        clearRequestedFlags();
        removeListeners();
    }

    protected void clearRequestedFlags() {
        mBatteryLevelRequested  = false;
    }

    @Override
    public void onGattResponse(@NonNull BluetoothDevice device, @NonNull BleGattResponse response) {
        // Nothing to do here !!! Everything is done using listeners on characteristic !
    }

    @Override
    public void onConnectionStateChanged(final BluetoothDevice device, @NonNull ConnectionState state) {
        switch(state) {
            case GATT_DISCONNECTED:
                // invalidate all characteristics and service
                Log.w(TAG, " - Invalidate BAS.");
                invalidateService();
                break;
        }
    }

    /* ****************************************************************************************** */
    /*                                                                                            */
    /*                                  PRIVATE SECTION                                           */
    /*                                                                                            */
    /* ****************************************************************************************** */
    private boolean readBatteryLevel() {
        if (mBatteryLevelChar == null)
            return false;

        if (!mBatteryLevelRequested) {
            mBatteryLevelRequested = true;
            mBleLib.queueCommand(new ReadCharValue(mBatteryLevelChar), () -> {
                mBatteryLevelRequested = false;
                return null;
            });
        }
        return mBatteryLevelRequested;
    }

    private void decodeBatteryLevel(BluetoothGattCharacteristic c, int status) {
        if (status == BluetoothGatt.GATT_SUCCESS) {
            mBatteryLevel = c.getIntValue(FORMAT_UINT8, 0);
            notifyListenerOnBatteryLevelChanged();
        }
    }

    private void notifyListenerOnBatteryLevelChanged() {
        for (BatteryServiceListener l: mListeners) {
            l.onBatteryLevelChanged(this, mBatteryLevel);
        }
    }

    private void notifyListenerOnBatteryLevelNotificationChanged() {
        for (BatteryServiceListener l: mListeners) {
            l.onBatteryLevelNotificationChanged(this, isBatteryLevelNotificationEnabled);
        }
    }

    private void resetParameter() {
        mBatteryLevel           = null;
        mBatteryLevelRequested  = false;
    }
}
