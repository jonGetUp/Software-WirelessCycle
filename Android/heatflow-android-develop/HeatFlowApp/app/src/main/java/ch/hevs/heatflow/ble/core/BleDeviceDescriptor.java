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
package ch.hevs.heatflow.ble.core;

import android.support.annotation.NonNull;
import android.support.annotation.Nullable;

import java.util.Locale;

import ch.hevs.ble.lib.scanner.BasicDiscoveredEntity;

/**
 * Descriptor of a discovered BLE device.
 * <p>
 * This descriptor contains basic information about a discovered BLE device, such as its Bluetooth advertising name,
 * its Bluetooth MAC address and the latest received RSSI value.
 *
 * @author Patrice Rudaz (patrice.rudaz@hevs.ch)
 * @version 1.0 - 2019/02/25
 */
public class BleDeviceDescriptor extends BasicDiscoveredEntity {
    private final static String TAG = "BleDeviceDescriptor";

    /**
     * Available advertising modes for the BLE device.
     */
    public enum AdvertisingMode {
        GeneralDiscoveryMode, // Default advertising mode (no whitelist)
        LimitedDiscoveryMode // Limited advertising mode to device in whitelist
    }

    /**
     * The advertising mode of the discovered BLE device.
     */
    private AdvertisingMode mAdvMode;

    /**
     * Create the descriptor of a discovered BLE device with general discovery mode (default) for
     * a BLE connected device.
     *
     * @param name       the name of the discovered BLE device (can be null if no public or not available)
     * @param address    the Bluetooth address as a {@code String} value
     * @param rssi       the current RSSI value or '0' if not available (signed value in dBm)
     * @param scanRecord the content of the advertisement record
     */
    public BleDeviceDescriptor(@Nullable String name, @NonNull String address, int rssi, @Nullable byte[] scanRecord) {
        this(name, address, rssi, scanRecord, AdvertisingMode.GeneralDiscoveryMode);
    }

    /**
     * Create the descriptor of a discovered BLE device with a specific discovery mode, which
     * could be Limited or General.
     *
     * @param name       the name of the discovered BLE device (can be null if no public or not available)
     * @param address    the Bluetooth address as a {@code String} value
     * @param rssi       the current RSSI value or '0' if not available (signed value in dBm)
     * @param scanRecord the content of the advertisement record
     * @param advMode    the device is in LE general or limited discovery mode
     */
    public BleDeviceDescriptor(@Nullable String name, @NonNull String address, int rssi, @Nullable byte[] scanRecord, @NonNull AdvertisingMode advMode) {
        super(name, address, rssi, scanRecord);
        mAdvMode = advMode;
    }

    /**
     * Create an empty and invalid descriptor.
     */
    public BleDeviceDescriptor() {
        this(null, "", 0, null);
    }

    /**
     * @param mode the advertising mode of the device. Must be read from the ADV flags
     */
    public void setAdvertisingMode(AdvertisingMode mode) {
        mAdvMode = mode;
    }

    /**
     * @return the advertising discovery mode of the BLE device
     */
    public AdvertisingMode getAdvertisingMode() {
        return mAdvMode;
    }

    @Override
    public String toString() {
        return String.format(Locale.getDefault(), "Device '%s' [%s] - %s", name(), address(), mAdvMode.name());
    }
}
