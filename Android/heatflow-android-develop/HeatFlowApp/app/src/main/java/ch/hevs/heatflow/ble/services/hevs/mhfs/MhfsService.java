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
package ch.hevs.heatflow.ble.services.hevs.mhfs;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.support.annotation.NonNull;
import android.util.Log;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Timer;
import java.util.TimerTask;
import java.util.UUID;

import ch.hevs.ble.lib.core.BleService;
import ch.hevs.ble.lib.events.commands.WriteCharConfig;
import ch.hevs.ble.lib.events.commands.WriteCharValue;
import ch.hevs.ble.lib.events.responses.CharDescWritten;
import ch.hevs.ble.lib.events.responses.OnBleCharacteristicChanged;
import ch.hevs.ble.lib.events.responses.OnBleCharacteristicDescriptorChanged;
import ch.hevs.heatflow.ble.error.MhfsAdsConfigurationError;
import ch.hevs.heatflow.ble.error.MhfsAdsConfigurationTimeoutError;
import ch.hevs.heatflow.ble.error.MhfsAdsValuesError;
import ch.hevs.heatflow.ble.error.MhfsCommandTimeoutError;
import ch.hevs.heatflow.ble.utils.HfBleConstants;
import ch.hevs.lib.soprod.watchkit.implementation.ble.services.DefaultService;

public class MhfsService extends DefaultService {
    private final static String TAG = "MhfsService";

    public static final byte MHFS_CMD_STOP                  = 0;
    public static final byte MHFS_CMD_START                 = 1;
    public static final byte MHFS_CMD_SINGLE_ADC            = 2;
    public static final byte MHFS_CMD_CONTINUOUS_ADC        = 3;
    public static final byte MHFS_CMD_TEMPERATURE           = 4;



    private BluetoothGattCharacteristic mConfigurationChar;
    private static final int MHFS_ADS_CONFIGURATION_SIZE    = 3;

    private BluetoothGattCharacteristic mCommandChar;
    private static final int MHFS_COMMAND_SIZE              = 1;

    private BluetoothGattCharacteristic mAdsValuesChar;
    private static final int MHFS_ADS_VALUES_SIZE           = 5;
    private boolean                     mIsAdsValuesNotificationEnabled;


    private Map<UUID,ByteBuffer> mBleQueue = new HashMap<>();


    /* ******************************************************************************************** */
    /*                                                                                              */
    /* Constructor's Section                                                                        */
    /*                                                                                              */
    /* ******************************************************************************************** */
    /** constructor */
    public MhfsService(BleService bleLibService) {
        super(bleLibService);
        invalidateService();    // init

        mResponseTimer = new Timer();
    }


    /* ******************************************************************************************** */
    /*                                                                                              */
    /* Listener's Section                                                                           */
    /*                                                                                              */
    /* ******************************************************************************************** */
    @NonNull
    private final List<MhfsServiceListener> mListeners = new LinkedList<>();

    /**
     * Register an observer. Do nothing if the observer is already registered.
     *
     * @param listener the observer to register has an observer
     */
    public void addListener(@NonNull MhfsServiceListener listener) {
        if (mListeners.contains(listener))
            return;

        mListeners.add(listener);
    }

    /**
     * Unregister the observer.
     *
     * @param listener the observer to unregister
     */
    public void removeListener(@NonNull MhfsServiceListener listener) {
        mListeners.remove(listener);
    }

    /**
     * Removed / unregister all the observers.
     */
    public void removeListeners() {
        mListeners.clear();
    }


    /* ******************************************************************************************** */
    /*                                                                                              */
    /* Getter's Section                                                                             */
    /*                                                                                              */
    /* ******************************************************************************************** */

    public boolean isAdsValuesNotificationEnabled() {
        return mIsAdsValuesNotificationEnabled;
    }

    /* ******************************************************************************************** */
    /*                                                                                              */
    /* DefaultService Implementation                                                                */
    /*                                                                                              */
    /* ******************************************************************************************** */
    @Override
    public void discoverService(@NonNull BluetoothGattService service) {
        if (mService == null) {
            mService = service;
            Log.i(TAG, " - SWS service available.");

            // Extract the characteristic from the service
            mConfigurationChar  = getCharacteristic(mService, MhfsUuid.CHAR_ADS_CONFIGURATION);
            mBleLib.addOnBleCharacteristicWrittenListener(mConfigurationChar, new OnBleCharacteristicChanged<BluetoothGattCharacteristic, Integer>() {
                @Override
                public Void call() {
                    // ToDo
                    return null;
                }
            });

            mCommandChar        = getCharacteristic(mService, MhfsUuid.CHAR_MHFS_CMD);
            mBleLib.addOnBleCharacteristicWrittenListener(mCommandChar, new OnBleCharacteristicChanged<BluetoothGattCharacteristic, Integer>() {
                @Override
                public Void call() {
                    decodeCommandCharValue(getCharacteristic(), getStatus());
                    return null;
                }
            });

            mAdsValuesChar      = getCharacteristic(mService, MhfsUuid.CHAR_MHFS_ADS_VAL);
            mBleLib.addOnBleCharacteristicChangedListener(mAdsValuesChar, new OnBleCharacteristicChanged<BluetoothGattCharacteristic, Integer>() {
                @Override
                public Void call() {
                    if (getStatus() == BluetoothGatt.GATT_SUCCESS) {
                        notifyOnAdsValuesChanged(ByteBuffer.wrap(getCharacteristic().getValue()));
                    }
                    return null;
                }
            });
            mBleLib.addOnCharacteristicDescriptorChangedListener(mAdsValuesChar, new OnBleCharacteristicDescriptorChanged<CharDescWritten.CCCDValue, Integer>() {
                @Override
                public Void call() {
                    mIsAdsValuesNotificationEnabled = isNotificationEnabled();
                    notifyOnAdsValuesNotificationChanged();
                    return null;
                }
            });
        }
    }

    /**
     * Invalidate the current service by setting all characteristics and service to `null`
     */
    @Override
    protected void invalidateService()  {
        // invalidate all characteristics
        mConfigurationChar  = null;
        mCommandChar        = null;
        mAdsValuesChar      = null;

        mBleQueue.clear();
        super.invalidateService();

        mIsAdsValuesNotificationEnabled = false;

        clearRequestedFlags();
        removeListeners();
        invalidateResponseTimerTask();
    }

    @Override
    protected void clearRequestedFlags() {
    }


    /* ******************************************************************************************** */
    /*                                                                                              */
    /* Override's Section                                                                           */
    /*                                                                                              */
    /* ******************************************************************************************** */
    @Override
    public void onConnectionStateChanged(final BluetoothDevice device, @NonNull ConnectionState state) {
        switch(state) {
            case GATT_DISCONNECTED:
                // invalidate all characteristics and service
                Log.w(TAG, " - Invalidate SNS service");
                invalidateService();
                break;
        }
    }


    /* ******************************************************************************************** */
    /*                                                                                              */
    /* Public's Section                                                                             */
    /*                                                                                              */
    /* ******************************************************************************************** */
    /**
     * Will send any data to the widget
     *
     * @param data  The new value to be displayed
     *
     * @return {@code true} if the BLE command could be added to the BLE command's queue and
     *         {@code false} otherwise
     */
    public boolean setCommand(Byte data) {
        if (mCommandChar == null) {
            Log.e(TAG, "NULL pointer on Command characteristic!");
            return false;
        }

        byte arrData[] = new byte[1];
        arrData[0] = (byte) data;
        ByteBuffer value = ByteBuffer.wrap(arrData);
        mBleQueue.put(MhfsUuid.CHAR_MHFS_CMD, value);
        return sendToBle();
    }


    /**
     * Enable or disable the notifications on the Button State's characteristic.
     *
     * @param enable Enable/disable configuration
     *
     * @return {@code true} if the BLE command could be added to the BLE command's queue and
     *         {@code false} otherwise
     */
    public boolean enableAdsValuesNotification(boolean enable) {
        if (mAdsValuesChar == null) {
            Log.e(TAG, "NULL pointer on ADS Values characteristic!");
            return false;
        }


        if ((enable && !mIsAdsValuesNotificationEnabled) || (!enable &&  mIsAdsValuesNotificationEnabled)) {
            mBleLib.queueCommand(WriteCharConfig.setNotification(mAdsValuesChar, enable), null);
        } else {
            Log.i(TAG, String.format(Locale.getDefault(), "Notification for ADS Values already %s, nothing to do ...", (enable ? "ENABLED":"DISABLED")));
        }
        return true;
    }


    /* ******************************************************************************************** */
    /*                                                                                              */
    /* Private's Section                                                                            */
    /*                                                                                              */
    /* ******************************************************************************************** */
    private boolean mIsTransmitting = false;


    /** NFC Time Response */
    private Timer mResponseTimer;

    // This timer task handle the callback error in case we receive no confirmation from the watch
    // It decides too when to send the next widget data
    private ResponseTimerTask mResponseTimerTask;

    // ---------------------------------------------------------------------------------------------
    private class ResponseTimerTask extends TimerTask {
        private BluetoothGattCharacteristic _gattChar = null;

        public ResponseTimerTask(BluetoothGattCharacteristic c) {
            _gattChar = c;
        }

        @Override
        public void run() {
            synchronized (this) {
                onResponseTimeout(_gattChar);
            }
        }
        public boolean cancel() {
            super.cancel();
            Log.d(TAG, "MHFS Response Timer task cancelled");
            return true;
        }
    }

    // ---------------------------------------------------------------------------------------------
    private boolean sendToBle() {
        if (mBleQueue.size() != 0 && !mIsTransmitting) {
            mIsTransmitting   = true;

            UUID charUuid    = null;
            if (mBleQueue.containsKey(MhfsUuid.CHAR_MHFS_CMD)) {
                charUuid = MhfsUuid.CHAR_MHFS_CMD;
            }
            else if (mBleQueue.containsKey(MhfsUuid.CHAR_ADS_CONFIGURATION)) {
                charUuid = MhfsUuid.CHAR_ADS_CONFIGURATION;
            }

            if (charUuid != null) {
                ByteBuffer data = mBleQueue.remove(charUuid);
                BluetoothGattCharacteristic c = mConfigurationChar;

                if (charUuid.equals(MhfsUuid.CHAR_MHFS_CMD))
                    c = mCommandChar;

                mBleLib.queueCommand(new WriteCharValue(c, data.array(), WRITE_REQUEST), () -> {
                    mIsTransmitting = false;
                    return null;
                });
                mResponseTimerTask = new ResponseTimerTask(c);
                mResponseTimer.schedule(mResponseTimerTask, HfBleConstants.HF_MHFS_RESPONSE_TIMEOUT_MS);
                return true;
            }

        }
        return false;
    }

    // ---------------------------------------------------------------------------------------------
    void onResponseTimeout(BluetoothGattCharacteristic c) {
        if (mIsTransmitting) {
            mIsTransmitting = false;

            if (c.getUuid().equals(MhfsUuid.CHAR_ADS_CONFIGURATION)) {
                for (MhfsServiceListener l : mListeners) {
                    l.onAdsConfigurationChanged(this, (byte) -1, (short) -1, new MhfsAdsConfigurationTimeoutError(HfBleConstants.HF_MHFS_RESPONSE_TIMEOUT_MS));
                }
            }

            else if (c.getUuid().equals(MhfsUuid.CHAR_MHFS_CMD)) {
                for (MhfsServiceListener l : mListeners) {
                    l.onCommandChanged(this, (byte) -1, new MhfsCommandTimeoutError(HfBleConstants.HF_MHFS_RESPONSE_TIMEOUT_MS));
                }
            }

            // process next command
            sendToBle();
        }
    }

    // ---------------------------------------------------------------------------------------------
    private void notifyOnAdsValuesNotificationChanged() {
        for (MhfsServiceListener l : mListeners) {
            l.onAdsValuesNotificationChanged(this, mIsAdsValuesNotificationEnabled);
        }
    }


    // ---------------------------------------------------------------------------------------------
    private void notifyOnAdsValuesChanged(ByteBuffer bufferedVal) {

        Error error     = null;
        final ByteBuffer val = ByteBuffer.wrap(bufferedVal.array());
        val.order(ByteOrder.LITTLE_ENDIAN);

        byte index      = (byte) -1;
        byte mux        = (byte) -1;
        byte pga        = (byte) -1;
        boolean isFloat = false;
        Integer value   = null;


        if (val.remaining() == MHFS_ADS_VALUES_SIZE) {
            byte config = val.get();

            index       = (byte) ( config       & 0x01);
            mux         = (byte) ((config >> 1) & 0x07);
            pga         = (byte) ((config >> 4) & 0x07);
            isFloat     = (byte) ((config >> 7) & 0x01) == 1;
            value       = val.getInt();
        } else {
            error = new MhfsAdsValuesError("Invalid size");
        }

        for (MhfsServiceListener l : mListeners) {
            l.onAdsValuesChanged(this, index, mux, pga, isFloat, value, error);
        }
    }

    // ---------------------------------------------------------------------------------------------
    private void notifyOnAdsConfigurationChanged(ByteBuffer data) {

        // Schedule a timeout for the reception in case some packets were dropped
        // The task will reset mResponseSizeReceived so the next packet will start as new
        invalidateResponseTimerTask();

        data.order(ByteOrder.LITTLE_ENDIAN);

        Error error  = null;
        Byte  index  = -1;
        Short config = -1;
        if (data.remaining() != MHFS_ADS_CONFIGURATION_SIZE) {
            error = new MhfsAdsConfigurationError("Size not valid");
        }

        else {
            // Decode index
            index  = data.get();
            config = data.getShort();

        }

        // Notify with received data
        for (MhfsServiceListener l : mListeners) {
            l.onAdsConfigurationChanged(this, index, config, error);
        }
    }

    // ---------------------------------------------------------------------------------------------
    private void invalidateResponseTimerTask() {
        if (mResponseTimerTask != null) {
            mResponseTimerTask.cancel();
        }

    }

    // ---------------------------------------------------------------------------------------------
    private void decodeCommandCharValue(BluetoothGattCharacteristic c, Integer status) {
        if (status == BluetoothGatt.GATT_SUCCESS) {
            notifyOnAdsConfigurationChanged(ByteBuffer.wrap(c.getValue()));
        }
    }

}
