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
package ch.hevs.heatflow.ble.discovery;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.annotation.RequiresPermission;
import android.util.Log;

import java.util.LinkedList;
import java.util.List;
import java.util.Locale;

import ch.hevs.ble.lib.exceptions.BleScanException;
import ch.hevs.ble.lib.scanner.BleScanner;
import ch.hevs.ble.lib.scanner.DiscoveredEntity;
import ch.hevs.ble.lib.scanner.DiscoveryListener;
import ch.hevs.ble.lib.scanner.Scanner;

import ch.hevs.heatflow.ble.core.BleDeviceDescriptor;

/**
 * Bluetooth scanner used to discover available Soprod Watches nearby.
 * <p>
 *     Use the Android BLE API to discover Bluetooth devices, filtered on the advertised Manufactu
 *     Data.
 *     To be filtered out, the Bluetooth device must advertise some Manufacturer Data with a known 
 *     Company Id, provided by the Bluetooth SIG.
 * </p>
 * 
 * @author Patrice Rudaz (patrice.rudaz@hevs.ch)
 * @version 1.0 - 2017/06/15
 * */
public class HfDiscovery implements DiscoveryListener {
    private static final String TAG = "HfDiscovery";

    private static final Integer SCAN_STOPPED           = 0;
    private static final Integer SCAN_FOR_DFU_TARGET    = 1;
    private static final Integer SCAN_FOR_HEVS_DEVICE   = 2;
    private static final Integer SCAN_ON_ERROR          = -1;

    @NonNull
    private final Scanner mScanner; // BLE scanner use as an attribute to keep the constructor private

    @NonNull
    private final HfDiscoveryListener mHfListener;

    private Integer mScanMode = SCAN_STOPPED;

    private List<String> mDeviceNames = null;

    private final static int ADV_LIMITED_DISC_FLAGS = 0x05;
    private final static int ADV_GENERAL_DISC_FLAGS = 0x06;

    /**
     * Create a Soprod Watches Bluetooth scanner. The scanner is not started automatically.
     *
     * @param adapter  the Bluetooth adapter to use
     */
    public HfDiscovery(@NonNull BluetoothAdapter adapter, @NonNull HfDiscoveryListener listener) {
        mHfListener = listener;
        mScanner = new BleScanner(adapter, this);
    } 

    public boolean scanForTarget(@Nullable List<String> names, int timeoutMs) {
        mDeviceNames = names;
        mScanMode = SCAN_FOR_DFU_TARGET;
        if (names != null) {
            Log.d(TAG, String.format(Locale.getDefault(), " Name's list: size = %d. Timeout %d [s]", names.size(), timeoutMs / 1000));
        } else {
            Log.d(TAG, String.format(Locale.getDefault(), " Name's list is empty. Timeout %d [s]", timeoutMs / 1000));
        }

        return mScanner.startScanningFor(timeoutMs);
    }

    /**
     * Start scanning / discovering SOP Watches until the scanner is stopped.
     * <p>
     * Requires the {@link Manifest.permission#BLUETOOTH_ADMIN} permission.
     * The app must hold {@link Manifest.permission#ACCESS_COARSE_LOCATION ACCESS_COARSE_LOCATION} or
     * {@link Manifest.permission#ACCESS_FINE_LOCATION ACCESS_FINE_LOCATION} permission in order to get results.
     *
     * @return {@code true} if the scanning process has started successfully, {@code false} otherwise
     * @see #startScanningFor(int)
     */
    @RequiresPermission(Manifest.permission.BLUETOOTH_ADMIN)
    public boolean startScanning() {
        return startScanningFor(0);
    }

    /**
     * Start scanning / discovering SOP Watches and stop scanning after a timeout.
     * <p>
     *
     * @param timeoutMs the time after the scan will stop in milliseconds (zero or negative time is infinite)
     * @return {@code true} if the scanning process has started successfully, {@code false} otherwise
     */
    @RequiresPermission(Manifest.permission.BLUETOOTH_ADMIN)
    public boolean startScanningFor(int timeoutMs) {
        Log.d(TAG, String.format(Locale.getDefault(), " Timeout %d [s]", timeoutMs / 1000));
        mDeviceNames = null;
        mScanMode = SCAN_FOR_HEVS_DEVICE;

        return mScanner.startScanningFor(timeoutMs);
    }

    @Nullable
    public List<DiscoveredEntity> getBondedDevice(@Nullable String name) {

        List<DiscoveredEntity> bondedDevices = mScanner.retrieveBondedDevice("Sop");
        List<DiscoveredEntity> bondedNamedDevices = new LinkedList<>();

        if (bondedDevices != null) {
            if (name != null) {
                for (DiscoveredEntity de: bondedDevices) {
                    if (de.name().toLowerCase().contains(name.toLowerCase())) {
                        Log.d(TAG, String.format(Locale.getDefault(), " bonded device found with name %s", de.name()));
                        bondedNamedDevices.add(de);
                    }
                }
                return bondedNamedDevices;
            }
        }
        return bondedDevices;
    }

    /**
     * Stop scanning / discovering SOP Watches. Do nothing if not scanning.
     */
    @RequiresPermission(Manifest.permission.BLUETOOTH_ADMIN)
    public void stopScanning() {
        mScanner.stopScanning();
        mScanMode = SCAN_STOPPED;
    }

    /**
     * Indicates if a scan is in progress or not.
     *
     * @return {@code true} if a scan is in progress, {@code false} otherwise
     */
    public boolean isScanning() {
        return mScanner.isScanning();
    }


    public void entityFoundIndication(@NonNull Scanner scanner, @NonNull DiscoveredEntity discoveredEntity) {

        if (mScanMode.equals(SCAN_FOR_DFU_TARGET)) {
            if (mDeviceNames != null && mDeviceNames.size() > 0) {

                for (String name : mDeviceNames) {

                    if (name.toLowerCase().equals(discoveredEntity.name().toLowerCase())) {
                        mHfListener.onDeviceDiscovered(scanner, toDeviceDescriptor(discoveredEntity));
                    }
                }
            } else {
                mHfListener.onDeviceDiscovered(scanner, toDeviceDescriptor(discoveredEntity));
            }

        } else if (mScanMode.equals(SCAN_FOR_HEVS_DEVICE)) {
            ScanRecord record = ScanRecord.parseFromBytes(discoveredEntity.scanRecord());

            byte[] data = record.getManufacturerSpecificData(HfManufacturerData.BLE_HEVS_COMPANY_IDENTIFIER);
            if (data != null) {
                mHfListener.onSpecificDeviceFound(scanner, toDeviceDescriptor(discoveredEntity));
            }
        }
    }

    public void scanningStoppedIndication(@NonNull Scanner scanner) {
        mHfListener.onDiscoveryStopped(scanner);
        mScanMode = SCAN_STOPPED;
    }

    public void onScanningError(@NonNull Scanner scanner, @NonNull BleScanException throwable) {
        mHfListener.onDiscoveryError(scanner, throwable);
        mScanMode = SCAN_ON_ERROR;
    }

    private static BleDeviceDescriptor toDeviceDescriptor(DiscoveredEntity discoveredEntity) {
        final BleDeviceDescriptor ret = new BleDeviceDescriptor();
        ret.setInfo(discoveredEntity.name(), discoveredEntity.address());
        ret.setAdvertisementInfo(discoveredEntity.rssi(), discoveredEntity.scanRecord());

        // Parse the advertising frame and check the ADV flags to known the discoverable mode of the device
        BleDeviceDescriptor.AdvertisingMode advMode = BleDeviceDescriptor.AdvertisingMode.GeneralDiscoveryMode; // Default mode
        final ScanRecord rec = ScanRecord.parseFromBytes(discoveredEntity.scanRecord());
        if (rec != null && rec.getAdvertiseFlags() == ADV_LIMITED_DISC_FLAGS) {
            advMode = BleDeviceDescriptor.AdvertisingMode.LimitedDiscoveryMode;
        }

        ret.setAdvertisingMode(advMode);
        return ret;
    }
}
