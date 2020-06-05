package ch.hevs.heatflow.ble.core;

import android.Manifest;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGattService;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.annotation.RequiresPermission;
import android.util.Log;

import java.lang.reflect.Method;
import java.util.LinkedHashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Locale;

import ch.hevs.ble.lib.core.BleService;
import ch.hevs.ble.lib.core.BleServiceObserver;
import ch.hevs.ble.lib.events.commands.BleCommand;
import ch.hevs.ble.lib.events.commands.CommandStatus;
import ch.hevs.ble.lib.events.responses.BleGattResponse;
import ch.hevs.heatflow.ble.services.adopted.BatteryService;
import ch.hevs.heatflow.ble.services.adopted.BatteryServiceListener;
import ch.hevs.heatflow.ble.services.adopted.BtSigUuid;
import ch.hevs.heatflow.ble.services.adopted.DeviceInformationService;
import ch.hevs.heatflow.ble.services.adopted.DeviceInformationServiceListener;
import ch.hevs.heatflow.ble.services.hevs.mhfs.MhfsService;
import ch.hevs.heatflow.ble.services.hevs.mhfs.MhfsServiceListener;
import ch.hevs.heatflow.ble.services.hevs.mhfs.MhfsUuid;
import ch.hevs.heatflow.ble.utils.BleUtils;

import static ch.hevs.ble.lib.core.BleServiceObserver.BleAdapterState.ADAPTER_STATE_OFF;

public class BleDevice implements BleServiceObserver, BatteryServiceListener, DeviceInformationServiceListener, MhfsServiceListener {
    private final static String TAG = "BleDevice";

    private Context mApplicationContext = null;

    // Shared BLE service from the library. Must be connected by an Activity
    private BleService mBleLibrary = null;

    @Nullable private BleDeviceDescriptor mDescriptor;

    @NonNull private final List<BleDeviceListener> mListeners = new LinkedList<>();

    private boolean mBleServiceBound = false;

    // Actual connection's state of the BLE device
    private BleConnectionState mState = BleConnectionState.DISCONNECTED;

    // Code to manage Service lifecycle.
    private final ServiceConnection mServiceConnection = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            mBleLibrary = ((BleService.LocalBinder) service).getService();

            // The shared BLE service is now connected. Can be used by the watch. Specific to BleDeviceKit Library for Android !!
            setBleService();

            // Setup the Android shared BLE service...
            setupBleSharedService();

            mBleServiceBound = true;
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            mBleLibrary = null;
            mBleServiceBound = false;
        }
    };

    /**
     * GATT error code returned by the Soprod Watch if the link is not secured when writing a value
     * to a characteristic handle or its CCCD handle.
     */
    private final static int BLE_GATT_STATUS_ATTERR_INSUF_AUTHORIZATION = 0x08;

    /**
     * Mandatory Battery Service
     */
    @Nullable private BatteryService mBasService = null;

    /**
     * Mandatory Device Information Service
     */
    @Nullable private DeviceInformationService mDisService = null;

    /**
     * Mandatory MHFS Service (Monitor HeatFlow Service
     */
    @Nullable private MhfsService mMhfsService = null;


    /* ******************************************************************************************** */
    /*                                                                                              */
    /* Android BLE service Binder                                                                   */
    /*                                                                                              */
    /* ******************************************************************************************** */
    /**
     * Bind the Android shared BLE service. <b>Absolutely necessary</b>
     * @param context   The Android application context
     */
    public boolean bindBleService(@NonNull Context context) {
        Intent gattServiceIntent = new Intent(context, BleService.class);
        mApplicationContext = context;

        if (mBleServiceBound) {
            unbindBleService();
        }

        // Binding to the Android BLE Shared service
        return context.bindService(gattServiceIntent, mServiceConnection, Context.BIND_AUTO_CREATE);
    }

    /* ******************************************************************************************** */
    /*                                                                                              */
    /* Listener's Section                                                                           */
    /*                                                                                              */
    /* ******************************************************************************************** */
    /**
     * Register an observer. Do nothing if the observer is already registered.
     *
     * @param listener the observer to register has an observer
     */
    public void addListener(@NonNull BleDeviceListener listener) {
        if (mListeners.contains(listener))
            return;

        mListeners.add(listener);
    }

    /**
     * Unregister the observer.
     *
     * @param listener the observer to unregister
     */
    public void removeListener(@NonNull BleDeviceListener listener) {
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
    /* GETTER/SETTER'S SECTION                                                                      */
    /*                                                                                              */
    /* ******************************************************************************************** */
    /**
     * Update the current device descriptor.
     *
     * @param desc the Soprod Watch descriptor of the Soprod watch
     */
    public void setDescriptor(@NonNull BleDeviceDescriptor desc) {
        mDescriptor = desc;
    }

    /**
     * Retrieves the current value of the RSSI (Received Strength Signal Indication)
     */
    public int getRSSI() {
        return mDescriptor.rssi();
    }

    /**
     * @return the current connection's state...
     */
    public BleConnectionState getConnectionState() {
        return mState;
    }

    /**
     * Check if the Bluetooth Soprod watch is connected or not.
     * <p>
     * To send any BLE request to the Soprod watch, it must be connected.
     *
     * @return  {@code true} if the Soprod Watch instance is {@link BleConnectionState#INITIALIZING}
     *          or {@link BleConnectionState#CONNECTED_READY}, and {@code false} otherwise.
     *          The method also returns {@code false} if the watch is not available.
     */
    public boolean isConnected() {
        // The shared BLE service must be available and the Soprod watch must be connected (ready).
        // If not all mandatory services are found, the Soprod watch state is 'error'.
        if (!isAvailable())
            return false;

        // The Soprod watch is either connected or ready
        return (mState == BleConnectionState.INITIALIZING) || (mState == BleConnectionState.CONNECTED_READY);
    }

    /**
     * Check if the Soprod Watch instance is ready to be used.
     *
     * @return  {@code true} if the Soprod watch is ready to be used
     *          ({@link BleConnectionState#CONNECTED_READY}), and {@code false} otherwise.
     */
    public boolean isReady() {
        // The shared BLE service must be available and the Soprod watch must be connected (ready).
        // If not all mandatory services are found, the Soprod watch state is 'error'.
        if (!isAvailable())
            return false;

        // The Soprod watch must be connected and in the ready state
        return (mState == BleConnectionState.CONNECTED_READY);
    }

    /**
     * Check if the Soprod watch is disconnected or not.
     *
     * @return {@code true} if the Soprod Watch is disconnected, {@code false} otherwise
     * @see BleDevice#isConnected()
     */
    public boolean isDisconnected() {
        return !isConnected();
    }


    @Override
    public void onGattResponse(@NonNull BluetoothDevice bluetoothDevice, @NonNull BleGattResponse response) {
    }

    @Override
    public void onCommandError(BluetoothDevice device, @NonNull BleCommand command, @NonNull CommandStatus error) {
        Log.e(TAG, String.format(Locale.getDefault(), "onCommandError() > cmd=%s, error=%s", command, error));
        // TODO: handle error here...

        switch(error) {
            case InternalError:
                // At this point we need to secure the Bluetooth link and ask to be bonded with the connected device.
                // This is not done automatically by the Android Bluetooth stack. Start the bonding (pairing) process with
                // the remote device. Asynchronous call, status updated in the `onBondingStateChanged` method.
                Log.e(TAG, String.format(Locale.getDefault(), "onCommandError() > %s is %s...", device.getName(), BleUtils.bondStateToString(device.getBondState())));
                //pairDevice(device);

                // Something wrong...
                //disconnect();
                break;

            default:
                break;
        }
    }

    @Override
    public void onGattError(BluetoothDevice device, @NonNull BleGattResponse response, int status) {
        // FIXME: checks other errors
        if (status == BLE_GATT_STATUS_ATTERR_INSUF_AUTHORIZATION) {
            // TODO: handle other errors here...
            Log.e(TAG, String.format(Locale.getDefault(), "onGattError, status=BLE_GATT_STATUS_ATTERR_INSUF_AUTHORIZATION, error=%s", response));
            // At this point we need to secure the Bluetooth link and ask to be bonded with the connected device.
            // This is not done automatically by the Android Bluetooth stack. Start the bonding (pairing) process with
            // the remote device. Asynchronous call, status updated in the `onBondingStateChanged` method.
            pairDevice(device);

            return; // The error has been handled
        }

        // TODO: handle other errors here...
        Log.e(TAG, String.format(Locale.getDefault(), "onGattError, status=%d, error=%s", status, response));

    }

    @Override
    public void onConnectionError(BluetoothDevice bluetoothDevice, @NonNull ConnectionErrorState connectionErrorState) {

    }


    /* ******************************************************************************************** */
    /*                                                                                              */
    /* BLE Service observer                                                                         */
    /*                                                                                              */
    /* ******************************************************************************************** */
    @Override
    public void onConnectionStateChanged(BluetoothDevice bluetoothDevice, @NonNull ConnectionState state) {
        switch (state) {
            case GATT_DISCONNECTED:
                if (state.statusCode() != 0)
                    Log.w(TAG, String.format(Locale.getDefault(), "%s disconnected, status=%d", mDescriptor, state.statusCode()));
                else
                    Log.d(TAG, String.format(Locale.getDefault(), "User disconnected from %s", mDescriptor));

                // Return the disconnect error code
                final BleConnectionState disconnectState = BleConnectionState.DISCONNECTED.setStatusCode(state.statusCode());
                notifyNewBleState(disconnectState);
                unbindBleService();
                break;
            case GATT_CONNECTING:
                // Connection to the Soprod Watch in progress - must wait until the services are discovered
                notifyNewBleState(BleConnectionState.CONNECTING);
                break;
            case GATT_CONNECTED:
                // At this point a Soprod watch can be bonded and connected, but the services have not been discovered.
                // We should wait `GATT_SERVICES_DISCOVERED` before reading or writing any data to the Soprod Watch.
                Log.d(TAG, "Connected to " + mDescriptor);

                // Valid Soprod Watch found but not yet ready
                notifyNewBleState(BleConnectionState.INITIALIZING);
                break;

            case GATT_SERVICES_DISCOVERING:
                Log.d(TAG, "Discovering services for " + mDescriptor.name());

                break;

            case GATT_SERVICES_DISCOVERED:
                Log.d(TAG, "Service discovered for " + mDescriptor.name());
                if (!lookupForMandatoryServices()) {
                    // Indicate a connection error if no the mandatory Bluetooth services are not available.
                    notifyNewBleState(BleConnectionState.ERROR_DEVICE_NOT_VALID);
                } else {

                    // Enable the mandatory notifications on the connected Soprod watch. This will make the Soprod watch
                    // ready once the output characteristic descriptor has been written.

                    // If the link is not secured or the pairing is invalid, the error request will return an error code. The
                    // link will be secure in the error callback. If the link is already secured (from a previous connection),
                    // The Soprod watch will be ready and no other operations are required. It will be ready as soon as the write
                    // operation is completed successfully.
                    enableMandatoryNotifications();
                }
                break;
        }
    }

    @Override
    public void onAdapterStateChanged(@NonNull BleAdapterState bleAdapterState) {
        Log.d(TAG, "onAdapterStateChanged() > Bluetooth Adapter state changed: " + bleAdapterState.name());
        try {
            notifyNewBleAdapterStateChanged(bleAdapterState);

            if (bleAdapterState.equals(ADAPTER_STATE_OFF) && mBleServiceBound) {

                /* try */
                disconnect();
                mState = BleConnectionState.DISCONNECTED;

                //Intent gattServiceIntent = new Intent(mApplicationContext, BleService.class);
                //mApplicationContext.stopService(gattServiceIntent);
                mApplicationContext.unbindService(mServiceConnection);
                mBleServiceBound = false;
            }
        } catch (Exception e) {
            Log.e(TAG, String.format(Locale.getDefault(), "Exception on gattServiceIntent ! Exception: %s", e.getLocalizedMessage()));
        }
    }

    @Override
    public void onBondingStateChanged(@NonNull BluetoothDevice device, @NonNull BondingState state) {
        Log.d(TAG, "Bonding state changed: " + state.name());

        if (state == BondingState.BONDED) {
            // Once we are bonded with the Soprod Watch, we have to enable the output notifications again because the
            // previous write operation probably failed.
            enableMandatoryNotifications();
        }
    }


    // ********************************************************************************************
    // Listener's notification on function changes...
    // ********************************************************************************************
    // Notify the listener that the mBleLibrary service is ready (connected)
    private void notifyListenerOnBleDeviceServiceReady() {
        // Loop on each listener
        for (BleDeviceListener listener : mListeners) {
            listener.onBleServiceReady();
        }
    }



    /* ******************************************************************************************** */
    /*                                                                                              */
    /* Public Section                                                                               */
    /*                                                                                              */
    /* ******************************************************************************************** */
    /**
     * Check if the Soprod watch is ready to accept a new connection and Read/Write requests.
     * <p>
     * Check if the shared BLE service is connected and available. If no BLE service is connect with
     * the Soprod watch, all BLE requests are ignored.
     *
     * @return {@code true} if the BLE service is bound, {@code false} otherwise
     */
    public boolean isAvailable() {
        // The Soprod watch is ready when the shared BLE service is connected and available
        return (mBleLibrary != null && mBleLibrary.isBleAdapterReady());
    }

    /**
     * This method tries to connect to the BLE Soprod watch using its address stored in the
     * {@link BleDeviceDescriptor}.
     * <p>
     * It ignores the connection request if already connected or if the BLE service is not connected.
     * The {@link BleDeviceListener#onBleDeviceConnected(BleDevice)} method is called when the
     * BLE device is successfully connected.
     *
     * @param timeoutMs maximum time to wait to be connected. If this timeout is reached,
     *                  the connection error callback will be called
     * @return {@code true} if the connection request succeeds, {@code false} otherwise.
     */
    public boolean connect(int timeoutMs) {
        Log.d(TAG, "Connection request to " + mDescriptor);

        if (isAvailable() && mDescriptor.address() != null) {
            // Setup the Mandatory service
            setMandatoryService(null);

            // Connect to the watch
            if (mBleLibrary.isBleAdapterReady()) {
                final String bleAddress = mDescriptor.address();
                return mBleLibrary.connect(bleAddress, timeoutMs);
            }
        } else {
            // Ignore the connection request if no descriptor is available or if the Bluetooth state is invalid
            Log.w(TAG, " - invalid descriptor");
        }
        return false;
    }

    /**
     * Disconnect and close the current BLE connection. Clear all pending commands if any.
     * <p>
     * Do nothing if the BLE service is not available or if not connected.
     * The peripheral state is updated when the disconnection request is terminated.
     *
     * @return {@code true} if the Bluetooth disconnection succeed, {@code false} otherwise
     */
    public boolean disconnect() {
        if (isAvailable()) {
            mBleLibrary.disconnect();
            return true;
        } else
            return false;
    }

    /**
     * If the BluetoothGATT is valid, it will clear its cache.
     */
    public void recoverDevice() {
        if (mBleLibrary != null) {
            mBleLibrary.clearBluetoothGattCache(null);
        }
    }
    /**
     * This function will send a BLE request to read the device information on the Soprod watch, such as
     * manufacturer name, software, firmware and hardware revision...
     * <p>
     * The result will be notified to the app through the callback method {@link BleDeviceListener#onDeviceInformationChanged(BleDevice)}
     */
    public void getDeviceInformation() {
        if (mDisService == null || !mDisService.readDeviceInformation()) {
            Log.e(TAG, "Read Device Information ERROR !");
        }
    }

    /**
     * @return The actual battery level in percent
     */
    public String getBatteryLevelInPercent() {
        int percent = getBatteryLevel();
        if (mBasService != null && percent != -1 )
            return String.format(Locale.getDefault(), "%d%%", percent);
        else
            return "-";
    }

    /**
     * @return The actual battery level as an int value or -1 if no value has already been read.
     */
    public int getBatteryLevel() {
        if (mBasService != null )
            return mBasService.getBatteryLevel();
        else
            return -1;
    }



    /* ******************************************************************************************** */
    /*                                                                                              */
    /* BleDeviceInformation Interface implementation                                                 */
    /*                                                                                              */
    /* ******************************************************************************************** */
    /**
     * Retrieves, from the Device Information's service, the last known value for the manufacturer's
     * name.
     * @return  The last read manufacturer's name.
     */
    public String getManufacturerName() {
        if (mDisService != null )
            return mDisService.getManufacturerName();
        else
            return "-";
    }

    /**
     * @return The user friendly name of the watch.
     */
    public String getBleName() {
        return mDescriptor.name();
    }

    /**
     * @return the Bluetooth address of the watch.
     */
    public String getBleAddress() {
        return mDescriptor.address();
    }

    /**
     * @return The model's number.
     */
    public String getModelNumber() {
        if (mDisService != null )
            return mDisService.getModelNumber();
        else
            return "-";
    }

    /**
     * @return The serial number
     */
    public String getSerialNumber() {
        if (mDisService != null )
            return mDisService.getSerialNumber();
        else
            return "-";
    }

    /**
     * @return The hardware's revision
     */
    public String getHardwareRevision() {
        if (mDisService != null )
            return mDisService.getHardwareRevision();
        else
            return "-";
    }

    /**
     * @return The firmware's revision
     */
    public String getFirmwareRevision() {
        if (mDisService != null )
            return mDisService.getFirmwareRevision();
        else
            return "-";
    }

    public void EnableMeasurement() {
        handleMhfsMeasurement(true);
    }

    public void disableMeasurement() {
        handleMhfsMeasurement(false);
    }


    /* ******************************************************************************************** */
    /*                                                                                              */
    /* Protected Section                                                                            */
    /*                                                                                              */
    /* ******************************************************************************************** */
    /**
     * Set the shared {@link BleService}.
     * <p>
     * Must be called by the main activity which manages the shared BLE service. Do nothing if the
     * service is {@code null}.
     * <p>
     * The method {@link BleDeviceListener#onBleDeviceReady(BleDevice)} is called when the BLE
     * service is available. Before this, it is not possible to connect the watch.
     * <p>
     * This is related to Android BLE stack implementation.
     *
     */
    protected void setBleService() {
        if (mBleLibrary == null)
            return; // No service connected now

        // Register has an observer to get notified and received events from the BLE service
        mBleLibrary.registerObserver(this);
        setMandatoryService(mBleLibrary);
    }

    /**
     * Absolutely necessary !
     */
    protected void setMandatoryService(@Nullable BleService service) {
        // Adopted services

        if (service != null) {
            mBasService     = new BatteryService(mBleLibrary);
            mDisService     = new DeviceInformationService(mBleLibrary);
            mMhfsService    = new MhfsService(mBleLibrary);
        }

        if (mBasService != null)
            mBasService.addListener(this);

        if (mDisService != null)
            mDisService.addListener(this);

        if (mMhfsService != null)
            mMhfsService.addListener(this);

    }

    /**
     * Enable or disable the notification on the output characteristic of the Soprod private SBS
     * service.
     * <p>
     * Do nothing if no Soprod watch is connected or if the Soprod private service is not
     * available.
     * <p>
     * Enabling the output notifications makes the connected Soprod Watch ready. If Bluetooth
     * notifications are not enabled, the Soprod Watch will be connected but not ready, this means
     * no data will be received from the Soprod Watch.
     *
     * @return {@code true} if success, {@code false} otherwise (if no Soprod watch connected)
     */
    protected boolean enableMandatoryNotifications() {
        // Ignore if the Soprod watch is not connected
        if (!isAvailable() || isDisconnected()) {
            Log.w(TAG, " - Soprod watch not connected");
            return false;
        }

        // DIS - Adopted Services
        if (mDisService == null) {
            Log.w(TAG, " - no DIS service available");
            return false;
        } else {
            mDisService.readDeviceInformation();
        }

        // BAS - Adopted Services
        if (mBasService == null) {
            Log.w(TAG, " - no BAS service available");
            return false;
        } else {
            mBasService.getBatteryLevel();
            mBasService.enableNotification(true);
        }

        if (mMhfsService == null) {
            Log.w(TAG, " - no MHFS service available");
            return false;
        } else {
            mMhfsService.enableAdsValuesNotification(true);
        }

        return true;
    }

    /**
     * Search for the mandatory private service on the connected device.
     *
     * @return {@code true} if all remote services and share found successfully, {@code false}
     * otherwise if the device is not considered as valid because at least one mandatory service is
     * missing.
     */
    protected boolean lookupForMandatoryServices() {
        // Get Adopted BLE services
        BluetoothGattService service = mBleLibrary.getSupportedGattService(BtSigUuid.SERVICE_BATTERY);
        if (service != null) {
            mBasService.discoverService(service);
        } else {
            Log.w(TAG, "- BAS not found !");
            return false;
        }

        service = mBleLibrary.getSupportedGattService(BtSigUuid.SERVICE_DEVICE_INFORMATION);
        if (service != null) {
            mDisService.discoverService(service);
        } else {
            Log.w(TAG, "- DIS not found !");
            return false;
        }

        service = mBleLibrary.getSupportedGattService(MhfsUuid.SERVICE_MHFS);
        if (service != null) {
            mMhfsService.discoverService(service);
        } else {
            Log.w(TAG, "- MHFS not found !");
            return false;
        }

        return true;
    }



    /* ******************************************************************************************** */
    /*                                                                                              */
    /* Private Section                                                                              */
    /*                                                                                              */
    /* ******************************************************************************************** */

    /* Setting up the Android Shared BLE service */
    private boolean setupBleSharedService() {
        // Enable verbose mode (available only if you use the debug BLE library)
        mBleLibrary.setVerbose(true);

        Log.d(TAG, "setupBleSharedService() > initialize BLE service");
        if (!mBleLibrary.initialize(mApplicationContext)) {
            Log.e(TAG, "Unable to initialize Bluetooth");
            return false;
        }

        notifyListenerOnBleDeviceServiceReady();
        return true;
    }

    /* unbind service ...*/
    private void unbindBleService() {
        if (mBleServiceBound) {
            try {
                mApplicationContext.unbindService(mServiceConnection);
            } catch (Exception e) {
                Log.d(TAG, "mServiceConnection already unbound or not bound...");
            }
            mBleServiceBound = false;
            mBleLibrary = null;
        }
    }

    /*
     * Save the new BLE Soprod watch state and notify the listeners about the new peripheral
     * device state.
     *
     * @param newState the new state of the peripheral device
     */
    private void notifyNewBleState(BleConnectionState newState) {
        Log.d(TAG, String.format(Locale.getDefault(), "Connection state old=%s, new=%s", mState, newState));
        mState = newState;

        switch (newState) {
            // Disconnected (with a status code)
            case DISCONNECTED:
                for (BleDeviceListener listener : mListeners) {
                    listener.onBleDeviceDisconnected(this, mState.statusCode());
                }
                break;

            // Connection in progress (not an error)
            case CONNECTING:
                for (BleDeviceListener listener : mListeners) {
                    listener.onBleDeviceConnecting(this);
                }
                break;

            // Connected and device valid (not an error). The device can be ready or not.
            case INITIALIZING:
                for (BleDeviceListener listener : mListeners) {
                    listener.onBleDeviceConnected(this);
                }
                break;

            case CONNECTED_READY:
                for (BleDeviceListener listener : mListeners) {
                    listener.onBleDeviceReady(this); // The connected Soprod Watch is now ready to use
                }
                break;

            case ERROR_DEVICE_NOT_VALID:
                // Trying the clear the GATT's cache
                Log.d(TAG, "ERROR_DEVICE_NOT_VALID -> Try to clear the Android BluetoothGatt's cache ...");
                mBleLibrary.clearBluetoothGattCache(null);

                for (BleDeviceListener listener : mListeners) {
                    listener.onBleDeviceConnectionError(this, newState);
                }
                break;

            // Unable to start a connection request or error during the connection
            default:
            case ERROR_TIMEOUT:
            case ERROR_DISCOVERING_SERVICES:
                for (BleDeviceListener listener : mListeners) {
                    listener.onBleDeviceConnectionError(this, newState);
                }

                // Set the device as disconnected after a connection error
                mState = BleConnectionState.DISCONNECTED;
                break;
        }
    }

    private void notifyNewBleAdapterStateChanged(BleAdapterState newState) {
        if (newState.equals(ADAPTER_STATE_OFF)) {

            for (BleDeviceListener l: mListeners) {
                l.onBleAdapterStateChanged(newState);
            }
        }
    }

    /*
     * This method can be used to gt the number of pending BLE command, which should still be sent
     * to the connected Soprod watch.
     * @return the number of pending commands which must be processed sent to the connected Soprod Watch
     */
    private int pendingCommands() {
        if (isAvailable())
            return mBleLibrary.pendingCommands();
        else
            return 0;
    }

    /*
     * Clear all the pending commands that must be sent to the connected Soprod watch.
     */
    private void clearPendingCommands() {
        if (isAvailable())
            mBleLibrary.clearPendingCommands();
    }

    /*
     * Will bond/pair the given BLE device.
     *
     * @param device    The BLE device to pair with the Android phone.
     */
    @RequiresPermission(Manifest.permission.BLUETOOTH_ADMIN)
    private void pairDevice(final @NonNull BluetoothDevice device) {
        if (mBleLibrary != null) {
            boolean bonded = mBleLibrary.createBond(device);
            Log.d(TAG, String.format(Locale.getDefault(), "Create bond with %s, %s", mDescriptor, bonded));
        }
    }

    /*
     * Should unpair the given BLE device.
     *
     * @param device    The BLE device to unpair
     */
    private void unpairDevice(final @NonNull BluetoothDevice device) {
        try {
            Log.d(TAG, String.format(Locale.getDefault(), "Remove bond with %s", mDescriptor));
            Method m = device.getClass().getMethod("removeBond", (Class[]) null);
            m.invoke(device, (Object[]) null);
        } catch (Exception e) {
            Log.e(TAG, e.getMessage());
        }
    }

    private boolean handleMhfsMeasurement(boolean enable) {
        if (mMhfsService != null )
            return mMhfsService.setCommand(enable ? MhfsService.MHFS_CMD_START:MhfsService.MHFS_CMD_STOP);
        else
            return false;
    }

    /* ******************************************************************************************** */
    /*                                                                                              */
    /* Battery Service interface implementation                                                     */
    /*                                                                                              */
    /* ******************************************************************************************** */
    @Override
    public void onBatteryLevelChanged(BatteryService service, int batteryLevel) {
        for (BleDeviceListener listener : mListeners) {
            listener.onBatteryLevelChanged(this, batteryLevel);
        }
    }

    @Override
    public void onBatteryLevelNotificationChanged(BatteryService service, boolean enable) {
        // Nothing to do here
    }


    /* ******************************************************************************************** */
    /*                                                                                              */
    /* Device Information Service interface implementation                                          */
    /*                                                                                              */
    /* ******************************************************************************************** */
    @Override
    public void onManufacturerNameRead(DeviceInformationService service, String manufacturerName) {
        for (BleDeviceListener listener : mListeners)
            listener.onDeviceInformationChanged(this);
    }

    @Override
    public void onModelNumberRead(DeviceInformationService service, String modelNumber) {
        for (BleDeviceListener listener : mListeners)
            listener.onDeviceInformationChanged(this);
    }

    @Override
    public void onSerialNumberRead(DeviceInformationService service, String serialNumber) {
        for (BleDeviceListener listener : mListeners)
            listener.onDeviceInformationChanged(this);
    }

    @Override
    public void onHardwareRevisionRead(DeviceInformationService service, String hardwareRevision) {
        for (BleDeviceListener listener : mListeners)
            listener.onDeviceInformationChanged(this);
    }

    @Override
    public void onFirmwareRevisionRead(DeviceInformationService service, String firmwareRevision) {
        for (BleDeviceListener listener : mListeners)
            listener.onDeviceInformationChanged(this);
    }


    /* ******************************************************************************************** */
    /*                                                                                              */
    /* Monitor HeatFlow Service interface implementation                                            */
    /*                                                                                              */
    /* ******************************************************************************************** */
    @Override
    public void onAdsConfigurationChanged(MhfsService service, Byte index, Short configuration, @Nullable Error error) {
        // Nothing to do
        if (error != null) {
            Log.e(TAG, String.format(Locale.getDefault(), "onAdsConfigurationChanged retrieved an error: %s", error.getLocalizedMessage()));
        }
    }

    @Override
    public void onCommandChanged(MhfsService service, Byte command, @Nullable Error error) {
        // Nothing to do
        if (error == null) {
            Log.d(TAG, String.format(Locale.getDefault(), "Command has been : %d", command));
        } else {
            Log.e(TAG, String.format(Locale.getDefault(), "onCommandChanged retrieved an error: %s", error.getLocalizedMessage()));
        }
    }

    @Override
    public void onAdsValuesChanged(MhfsService service, byte adsId, byte mux, byte pga, boolean isFloatValue, Integer value, @Nullable Error error) {
        if (error == null) {
            for (BleDeviceListener listener : mListeners)
                listener.onAdsValuesChanged(this, service, adsId, mux, pga, isFloatValue, value);
        } else {
            Log.e(TAG, String.format(Locale.getDefault(), "onAdsValuesChanged retrieved an error: %s", error.getLocalizedMessage()));
        }
    }

    @Override
    public void onAdsValuesNotificationChanged(MhfsService service, boolean enable) {
        // Nothing to do
        Log.d(TAG, String.format(Locale.getDefault(), "Notification has been %s for ADS values Char...", enable ? "enabled":"disabled"));
        if (enable) {
            for (BleDeviceListener listener : mListeners)
                listener.onBleDeviceReady(this);
        } else {
            for (BleDeviceListener listener : mListeners)
                listener.onBleDeviceConnected(this);
        }
    }
}
