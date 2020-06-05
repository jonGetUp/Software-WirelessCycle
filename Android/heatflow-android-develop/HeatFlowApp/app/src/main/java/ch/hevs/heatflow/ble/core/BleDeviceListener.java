package ch.hevs.heatflow.ble.core;

import android.support.annotation.BinderThread;
import android.support.annotation.NonNull;

import ch.hevs.ble.lib.core.BleServiceObserver;
import ch.hevs.heatflow.ble.services.hevs.mhfs.MhfsService;

public interface BleDeviceListener {

    /* Related to Android Service */

    /**
     * Called when the Android shared Bluetooth Adapter has been registered successfully as a
     * Service.
     */
    @BinderThread
    void onBleServiceReady();

    /**
     * Called When the local shared Bluetooth adapter has been shut OFF or turned ON...
     * <p>
     * {@code adapterState} can take the values TURN_ON or TURN_OFF.
     */
    @BinderThread
    void onBleAdapterStateChanged(@NonNull BleServiceObserver.BleAdapterState adapterState);


    /* Related to the BLE device connection states and status only. */

    /**
     * Called when the Soprod BLE watch becomes busy.
     * <p>
     * The watch becomes busy when the BLE stack needs to upgrade its connection's parameters to
     * become more reactive.
     *
     * @param device the current SopWatchInterface
     */
    @BinderThread
    void onBleDeviceBusy(@NonNull BleDevice device);

    /**
     * Called when BLE Device is ready.
     * <p>
     * The user can now request a Bluetooth connection. The Bluetooth Android adapter is ready.
     *
     * @param device the current BLE Device
     */
    @BinderThread
    void onBleDeviceReady(@NonNull BleDevice device);

    /**
     * Indicates when the connection process has started, after sending a connection request.
     *
     * @param device the current SopWatchInterface
     */
    @BinderThread
    void onBleDeviceConnecting(@NonNull BleDevice device);

    /**
     * The BLE Device is now connected.
     * <p>
     * The phone is now connected to the Bluetooth device. The connected device is a valid BLE Device.
     * <p>
     * When connected, the device can be ready or not. When ready, output notifications on the
     * BLE Device are enabled. If not ready, we will not be able to received any notifications from
     * the connected device. The device should be ready soon after connected. If the device is not
     * ready, data can be sent to the device, but not received.

     * @param device the current BLE Device
     */
    @BinderThread
    void onBleDeviceConnected(@NonNull BleDevice device);

    /**
     * Unable to connect and discover all required services on the specified Bluetooth device.
     * <p>
     * The BLE device is probably not a BLE Device. Mandatory services are not available or an error
     * occurred during the connection (timeout). The connection timeout can be set in the
     * {@link BleDevice#connect(int)} method.

     * @param device     the current BLE Device
     * @param errorState the current error state. Can be {@link BleConnectionState#ERROR_TIMEOUT}
     *                   if a timeout occurred when connecting or
     *                   {@link BleConnectionState#ERROR_DEVICE_NOT_VALID} if the mandatory services
     *                   are not available in the device.
     */
    @BinderThread
    void onBleDeviceConnectionError(@NonNull BleDevice device, BleConnectionState errorState);

    /**
     * The BLE Device is now disconnected.
     * <p>
     * Called if the Bluetooth connection has been closed by the user when calling the
     * {@link BleDevice#disconnect()} method or after a link loss.

     * @param device        The current BLE Device
     * @param statusCode    The disconnection status code
     */
    @BinderThread
    void onBleDeviceDisconnected(@NonNull BleDevice device, int statusCode);

    /**
     * TODO
     * @param device
     * @param adsId
     * @param mux
     * @param pga
     * @param isFloatValue
     * @param value
     */
    @BinderThread
    void onAdsValuesChanged(@NonNull BleDevice device, @NonNull MhfsService service, byte adsId, byte mux, byte pga, boolean isFloatValue, Integer value);


    // MARK: - Not defined yet in which function those information will integrate
    @BinderThread
    void onDeviceInformationChanged(@NonNull BleDevice device);

    @BinderThread
    void onBatteryLevelChanged(@NonNull BleDevice device, int batteryLevel);
}
