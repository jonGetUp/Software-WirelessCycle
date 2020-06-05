package ch.hevs.heatflow.ble.core;

import java.util.Locale;

/**
 * Enumerate the different available state of the BLE device instance in term of connection.
 *
 * @author Patrice Rudaz (patrice.rudaz@hevs.ch, patrice.rudaz@soprod.ch)
 * @version 1.0 - 2019/02/25
 */
public enum BleConnectionState {

    /* Connection states */

    /**
     * No active Bluetooth connection to the BLE device.
     * The error code is returned. It can be used to known if it is a link loss or a user request.
     * The code {@code 0} is a success (disconnect request from the user).
     */
    DISCONNECTED, // With a status code

    /**
     * Connection is in progress. Stay in the connecting state until all the remote services have
     * been discovered.
     * Switch to {@link #INITIALIZING} if a valid BLE device has been found, or to the error state
     * {@link #ERROR_DEVICE_NOT_VALID} if this is not a valid BLE device.
     * A timeout or an error could also been reported during the connection process.
     */
    CONNECTING,

    /**
     * A valid BLE device is connected and all remote services have been found. At this point, a
     * valid BLE device is connected and all the functions requested or disposed by the watch are
     * initializing.
     */
    INITIALIZING,

    /**
     * The mandatory notifications have been enabled on the SOP watch, which is now ready to be used.
     */
    CONNECTED_READY,


    /* Error states */
    /**
     * Mandatory remote services are not available on the connected device. The Bluetooth device
     * we try to connected to is not a valid BLE device. Some mandatory BLE services are missing.
     */
    ERROR_DEVICE_NOT_VALID, // Error state

    /**
     * Unable to connect to the requested BLE device after the defined amount of time.
     * A connection request has been sent, but the Bluetooth device does not accept the connection
     * before the specified timeout. A new connection request must be sent to try to connect again.
     * This error can occurs if the device goes out of range or is powered off during the connection
     * process.
     */
    ERROR_TIMEOUT, // Error state

    /**
     * An error occurred when discovering the services on the connected remote device.
     */
    ERROR_DISCOVERING_SERVICES; // Error state

    private int mStatusCode = 0;

    /**
     * Change the status code associated to the state ot indicate an error code or a status.
     *
     * @param statusCode teh error or status code corresponding to the state
     * @return the updated state
     */
    public BleConnectionState setStatusCode(int statusCode) {
        mStatusCode = statusCode;
        return this;
    }

    /**
     * Return the status code associated to the state to indicate an error code if required.
     *
     * @return the status code associated to the state
     */
    public int statusCode() {
        return mStatusCode;
    }

    @Override
    public String toString() {
        return String.format(Locale.US, "%s(%d)", name(), statusCode());
    }
}
