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
import java.util.Locale;

import ch.hevs.ble.lib.core.BleService;
import ch.hevs.ble.lib.events.commands.ReadCharValue;
import ch.hevs.ble.lib.events.responses.BleGattResponse;
import ch.hevs.ble.lib.events.responses.OnBleCharacteristicChanged;
import ch.hevs.lib.soprod.watchkit.implementation.ble.services.DefaultService;


/**
 * For Internal Use only !
 * @author Patrice Rudaz (patrice.rudaz@hevs.ch)
 * @version 1.0 - 2017/06/15
 */
public class DeviceInformationService extends DefaultService {
    private final static String TAG = "DeviceInfoService";

    // Manufacturer name
    private String mManufacturerName = null;

    // ModelNumber
    private String mModelNumber = null;

    // Serial Number
    private String mSerialNumber = null;

    // Hardware Revision
    private String mHardwareRevision = null;

    // Software Revision
    private String mSoftwareRevision = null;

    // Firmware Revision
    private String mFirmwareRevision = null;

    /** Manufacturer Name */
    private BluetoothGattCharacteristic mManufacturerNameChar;
    private boolean                     mManufacturerNameRequested;

    /** Model Number */
    private BluetoothGattCharacteristic mModelNumberChar;
    private boolean                     mModelNumberRequested;

    /** Serial Number */
    private BluetoothGattCharacteristic mSerialNumberChar;
    private boolean                     mSerialNumberRequested;

    /** Hardware Revision */
    private BluetoothGattCharacteristic mHardwareRevisionChar;
    private boolean                     mHardwareRevisionRequested;

    /** Software Revision */
    private BluetoothGattCharacteristic mSoftwareRevisionChar;
    private boolean                     mSoftwareRevisionRequested;

    /** Firmware Revision */
    private BluetoothGattCharacteristic mFirmwareRevisionChar;
    private boolean                     mFirmwareRevisionRequested;


    /* ******************************************************************************************** */
    /*                                                                                              */
    /*                                CONSTRUCTOR'S SECTION                                         */
    /*                                                                                              */
    /* ******************************************************************************************** */
    /** constructor */
    public DeviceInformationService(BleService bleLibService) {
        super(bleLibService);
        invalidateService();
    }


    /* ******************************************************************************************** */
    /*                                                                                              */
    /*                                  LISTENER'S SECTION                                          */
    /*                                                                                              */
    /* ******************************************************************************************** */
    @NonNull
    private final List<DeviceInformationServiceListener> mListeners = new LinkedList<>();

    /**
     * Register an observer. Do nothing if the observer is already registered.
     *
     * @param listener the observer to register has an observer
     */
    public void addListener(@NonNull DeviceInformationServiceListener listener) {
        if (mListeners.contains(listener))
            return;

        mListeners.add(listener);
    }

    /**
     * Unregister the observer.
     *
     * @param listener the observer to unregister
     */
    public void removeListener(@NonNull DeviceInformationServiceListener listener) {
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
    /*                                   GETTER'S SECTION                                           */
    /*                                                                                              */
    /* ******************************************************************************************** */
    /** Returns the Manufacturer name */
    public String getManufacturerName() {
        if (mManufacturerName != null)
           return mManufacturerName;

        readManufacturerName();
        return "-";
    }

    /** Returns the Model Number */
    public String getModelNumber() {
        if (mModelNumber != null)
            return mModelNumber;

        readModelNumber();
        return "-";
    }

    /** Returns the Serial Number */
    public String getSerialNumber()  {
        if (mSerialNumber != null)
            return mSerialNumber;

        readSerialNumber();
        return "-";
    }

    /** Returns the Hardware Revision */
    public String getHardwareRevision() {
        if (mHardwareRevision != null)
            return mHardwareRevision;

        readHardwareRevision();
        return "-";
    }

    /** Returns the Software Revision */
    public String getSoftwareRevision() {
        if (mSoftwareRevision != null)
            return mSoftwareRevision;

        readSoftwareRevision();
        return "-";
    }

    /** Returns the Firmware Revision */
    public String getFirmwareRevision() {
        if (mFirmwareRevision != null)
            return mFirmwareRevision;

        readFirmwareRevision();
        return "-";
    }


    /* ******************************************************************************************** */
    /*                                                                                              */
    /*                                   PUBLIC'S SECTION                                           */
    /*                                                                                              */
    /* ******************************************************************************************** */

    /** Read all device information */
    public boolean readDeviceInformation() {
        if (mManufacturerName != null)
            notifyListenerOnManufacturerNameRead();
        else if (!readManufacturerName())
            return false;

        if (mModelNumber != null)
            notifyListenerOnModelNumberRead();
        else if (!readModelNumber())
            return false;

        if (mSerialNumber != null)
            notifyListenerOnSerialNumberRead();
        else if (!readSerialNumber())
            return false;

        if (mHardwareRevision != null)
            notifyListenerOnHardwareRevisionRead();
        else if (!readHardwareRevision())
            return false;

        if (mSoftwareRevision != null)
            notifyListenerOnSoftwareRevisionRead();
        else if (!readSoftwareRevision())
            return false;

        if (mFirmwareRevision != null)
            notifyListenerOnFirmwareRevisionRead();
        else
            return readFirmwareRevision();

        return true;
    }

    public void discoverService(@NonNull BluetoothGattService service) {
        if (mService == null) {
            Log.i(TAG, "- DIS available !");
            mService                = service;

            // Extract the characteristic from the service
            mManufacturerNameChar   = getCharacteristic(mService, BtSigUuid.CHAR_MANUFACTURER_NAME);
            mModelNumberChar        = getCharacteristic(mService, BtSigUuid.CHAR_MODEL_NUMBER);
            mSerialNumberChar       = getCharacteristic(mService, BtSigUuid.CHAR_SERIAL_NUMBER);
            mHardwareRevisionChar   = getCharacteristic(mService, BtSigUuid.CHAR_HARDWARE_REVISION);
            mSoftwareRevisionChar   = getCharacteristic(mService, BtSigUuid.CHAR_SOFTWARE_REVISION);
            mFirmwareRevisionChar   = getCharacteristic(mService, BtSigUuid.CHAR_FIRMWARE_REVISION);

            // add listeners on Characteristics
            mBleLib.addOnBleCharacteristicChangedListener(mManufacturerNameChar, new OnBleCharacteristicChanged<BluetoothGattCharacteristic, Integer>() {
                @Override
                public Void call() {
                    mManufacturerName = decodeDeviceInformation(getCharacteristic(), getStatus());
                    Log.d(TAG, String.format(Locale.getDefault(), "Manufacturer name: %s", mManufacturerName));
                    notifyListenerOnManufacturerNameRead();
                    return null;
                }
            });
            mBleLib.addOnBleCharacteristicChangedListener(mModelNumberChar, new OnBleCharacteristicChanged<BluetoothGattCharacteristic, Integer>() {
                @Override
                public Void call() {
                    mModelNumber = decodeDeviceInformation(getCharacteristic(), getStatus());
                    Log.d(TAG, String.format(Locale.getDefault(), "Model Number: %s", mModelNumber));
                    notifyListenerOnModelNumberRead();
                    return null;
                }
            });
            mBleLib.addOnBleCharacteristicChangedListener(mSerialNumberChar, new OnBleCharacteristicChanged<BluetoothGattCharacteristic, Integer>() {
                @Override
                public Void call() {
                    mSerialNumber = decodeDeviceInformation(getCharacteristic(), getStatus());
                    Log.d(TAG, String.format(Locale.getDefault(), "Serial Number: %s", mSerialNumber));
                    notifyListenerOnSerialNumberRead();
                    return null;
                }
            });
            mBleLib.addOnBleCharacteristicChangedListener(mHardwareRevisionChar, new OnBleCharacteristicChanged<BluetoothGattCharacteristic, Integer>() {
                @Override
                public Void call() {
                    mHardwareRevision = decodeDeviceInformation(getCharacteristic(), getStatus());
                    Log.d(TAG, String.format(Locale.getDefault(), "Hardware Revision: %s", mHardwareRevision));
                    notifyListenerOnHardwareRevisionRead();
                    return null;
                }
            });
            mBleLib.addOnBleCharacteristicChangedListener(mSoftwareRevisionChar, new OnBleCharacteristicChanged<BluetoothGattCharacteristic, Integer>() {
                @Override
                public Void call() {
                    mSoftwareRevision = decodeDeviceInformation(getCharacteristic(), getStatus());
                    Log.d(TAG, String.format(Locale.getDefault(), "Software Revision: %s", mSoftwareRevision));
                    notifyListenerOnSoftwareRevisionRead();
                    return null;
                }
            });
            mBleLib.addOnBleCharacteristicChangedListener(mFirmwareRevisionChar, new OnBleCharacteristicChanged<BluetoothGattCharacteristic, Integer>() {
                @Override
                public Void call() {
                    mFirmwareRevision = decodeDeviceInformation(getCharacteristic(), getStatus());
                    Log.d(TAG, String.format(Locale.getDefault(), "Firmware Revision: %s", mFirmwareRevision));
                    notifyListenerOnFirmwareRevisionRead();
                    return null;
                }
            });

            resetParameter();
        }
    }

    /* ******************************************************************************************** */
    /*                                                                                              */
    /*                                  OVERRIDE'S SECTION                                          */
    /*                                                                                              */
    /* ******************************************************************************************** */
    /**
     * Invalidate the current service by setting all characteristics and service to `null`
     */
    @Override
    protected void invalidateService()  {
        super.invalidateService();

        // invalidate all characteristics
        mManufacturerNameChar       = null;
        mModelNumberChar            = null;
        mSerialNumberChar           = null;
        mHardwareRevisionChar       = null;
        mSoftwareRevisionChar       = null;
        mFirmwareRevisionChar       = null;

        resetParameter();
        clearRequestedFlags();
        removeListeners();
    }

    protected void clearRequestedFlags() {
        mManufacturerNameRequested  = false;
        mModelNumberRequested       = false;
        mSerialNumberRequested      = false;
        mHardwareRevisionRequested  = false;
        mSoftwareRevisionRequested  = false;
        mFirmwareRevisionRequested  = false;
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
                Log.w(TAG, " - Invalidate DIS.");
                invalidateService();
                break;
        }
    }


    /* ******************************************************************************************** */
    /*                                                                                              */
    /*                                   PRIVATE SECTION                                            */
    /*                                                                                              */
    /* ******************************************************************************************** */
    private boolean readManufacturerName() {
        if (mManufacturerNameChar == null)
            return false;

        if (!mManufacturerNameRequested) {
            mManufacturerNameRequested = true;
            mBleLib.queueCommand(new ReadCharValue(mManufacturerNameChar), () -> {
                mManufacturerNameRequested = false;
                return null;
            });
        }

        return mManufacturerNameRequested;
    }

    private boolean readModelNumber() {
        if (mModelNumberChar == null)
            return false;

        if (!mModelNumberRequested) {
            mModelNumberRequested = true;
            mBleLib.queueCommand(new ReadCharValue(mModelNumberChar), () -> {
                mModelNumberRequested = false;
                return null;
            });
        }

        return mModelNumberRequested;
    }

    private boolean readSerialNumber() {
        if (mSerialNumberChar == null)
            return false;

        if (!mSerialNumberRequested) {
            mSerialNumberRequested = true;
            mBleLib.queueCommand(new ReadCharValue(mSerialNumberChar), () -> {
                mSerialNumberRequested = false;
                return null;
            });
        }

        return mSerialNumberRequested;
    }

    private boolean readHardwareRevision() {
        if (mHardwareRevisionChar == null)
            return false;

        if (!mHardwareRevisionRequested) {
            mHardwareRevisionRequested = true;
            mBleLib.queueCommand(new ReadCharValue(mHardwareRevisionChar), () -> {
                mHardwareRevisionRequested = false;
                return null;
            });
        }

        return mHardwareRevisionRequested;
    }

    private boolean readSoftwareRevision() {
        if (mSoftwareRevisionChar == null)
            return false;

        if (!mSoftwareRevisionRequested) {
            mSoftwareRevisionRequested = true;
            mBleLib.queueCommand(new ReadCharValue(mSoftwareRevisionChar), () -> {
                mSoftwareRevisionRequested = false;
                return null;
            });
        }

        return mSoftwareRevisionRequested;
    }

    private boolean readFirmwareRevision() {
        if (mFirmwareRevisionChar == null)
            return false;

        if (!mFirmwareRevisionRequested) {
            mFirmwareRevisionRequested = true;
            mBleLib.queueCommand(new ReadCharValue(mFirmwareRevisionChar), () -> {
                mFirmwareRevisionRequested = false;
                return null;
            });
        }

        return mFirmwareRevisionRequested;
    }

    private void notifyListenerOnManufacturerNameRead() {
        for (DeviceInformationServiceListener l: mListeners) {
            l.onManufacturerNameRead(this, mManufacturerName);
        }
    }

    private void notifyListenerOnModelNumberRead() {
        for (DeviceInformationServiceListener l: mListeners) {
            l.onModelNumberRead(this, mModelNumber);
        }
    }

    private void notifyListenerOnSerialNumberRead() {
        for (DeviceInformationServiceListener l: mListeners) {
            l.onSerialNumberRead(this, mSerialNumber);
        }
    }

    private void notifyListenerOnHardwareRevisionRead() {
        for (DeviceInformationServiceListener l: mListeners) {
            l.onHardwareRevisionRead(this, mHardwareRevision);
        }
    }

    private void notifyListenerOnSoftwareRevisionRead() {
//        for (DeviceInformationServiceListener l: mListeners) {
//            l.onSoftwareRevisionRead(this, mSoftwareRevision);
//        }
    }

    private void notifyListenerOnFirmwareRevisionRead() {
        for (DeviceInformationServiceListener l: mListeners) {
            l.onFirmwareRevisionRead(this, mFirmwareRevision);
        }
    }

    private String decodeDeviceInformation(BluetoothGattCharacteristic c, Integer status) {
        if (status == BluetoothGatt.GATT_SUCCESS) {
            return c.getStringValue(0);
        }
        return "-";
    }

    private void resetParameter() {
        mManufacturerName           = null;
        mModelNumber                = null;
        mSerialNumber               = null;
        mHardwareRevision           = null;
        mSoftwareRevision           = null;
        mFirmwareRevision           = null;

        mManufacturerNameRequested  = false;
        mFirmwareRevisionRequested  = false;
        mHardwareRevisionRequested  = false;
        mModelNumberRequested       = false;
        mSerialNumberRequested      = false;
        mSoftwareRevisionRequested  = false;
    }
}
