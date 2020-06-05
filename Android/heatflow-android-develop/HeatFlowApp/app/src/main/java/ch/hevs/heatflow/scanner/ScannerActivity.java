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
package ch.hevs.heatflow.scanner;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothManager;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.ParcelUuid;
import android.support.annotation.NonNull;
import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import java.lang.reflect.Method;
import java.util.Locale;
import java.util.Set;
import java.util.UUID;

import ch.hevs.ble.lib.exceptions.BleScanException;
import ch.hevs.ble.lib.scanner.DiscoveredEntity;
import ch.hevs.ble.lib.scanner.OnEntityClickListener;
import ch.hevs.ble.lib.scanner.Scanner;

import ch.hevs.heatflow.BuildConfig;
import ch.hevs.heatflow.DefaultActivity;
import ch.hevs.heatflow.MainActivity;
import ch.hevs.heatflow.R;
import ch.hevs.heatflow.ble.core.BleDeviceDescriptor;
import ch.hevs.heatflow.ble.discovery.HfDiscovery;
import ch.hevs.heatflow.ble.discovery.HfDiscoveryListener;
import ch.hevs.heatflow.ble.utils.HfBleConstants;

public class ScannerActivity extends DefaultActivity implements OnEntityClickListener, HfDiscoveryListener {
    private final static String TAG = "ScanActivity";

    private static final int REQUEST_ENABLE_BT = 1;
    final private UUID BT_SPP_UUID = UUID.fromString("0001101-0000-1000-8000-00805F9B34FB");

    private HfDiscovery mScanner;
    private EntityAdapter mEntityAdapter;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_scanner);

        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        if (toolbar != null) {
            setSupportActionBar(toolbar);
            getSupportActionBar().setDisplayShowTitleEnabled(false);
        }

        // Displaying the application version and config
        final TextView tvVersion = (TextView) findViewById(R.id.tvAppVersion);
        tvVersion.setText(String.format(Locale.US, "Version %s (%d)", BuildConfig.VERSION_NAME, BuildConfig.VERSION_CODE));

        // Before using the Bluetooth, ask the user to allow the required permissions in order to use BLE
        // on Android M (API 23) and above. Only required with the new System Permissions.
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M)
            askRequiredBlePermissions();

        // Use this check to determine whether BLE is supported on the device.  Then you can
        // selectively disable BLE-related features.
        if (!getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE)) {
            Toast.makeText(this, R.string.ble_not_supported, Toast.LENGTH_SHORT).show();
            finish();
        }

        // Initializes a Bluetooth adapter.  For API level 18 and above, get a reference to
        // BluetoothAdapter through BluetoothManager.
        final BluetoothManager bluetoothManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        mBluetoothAdapter = bluetoothManager.getAdapter();
        // Checks if Bluetooth is supported on the device.
        if (mBluetoothAdapter == null) {
            Toast.makeText(this, R.string.error_bluetooth_not_supported, Toast.LENGTH_SHORT).show();
            finish();
        }

        // Initializes list view adapter
        mEntityAdapter = new EntityAdapter(this);

        RecyclerView rv = (RecyclerView) findViewById(R.id.devices_list);
        rv.setAdapter(mEntityAdapter);
        rv.setLayoutManager(new LinearLayoutManager(this));

        /** ***************************************************************************************
         * Create a Soprod Watch scanner and register to be notified when Soprod Watches have been
         * found.
         *  *************************************************************************************** */
        mScanner = new HfDiscovery(mBluetoothAdapter, this);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.menu_scanner, menu);

        // Update the menu depending on the scanner state
        if (!mScanner.isScanning()) {
            menu.findItem(R.id.menu_stop_scan).setVisible(false);
            menu.findItem(R.id.menu_start_scan).setVisible(true);
            menu.findItem(R.id.menu_refresh).setVisible(false);
            menu.findItem(R.id.menu_refresh).setActionView(null);
        } else {
            menu.findItem(R.id.menu_stop_scan).setVisible(true);
            menu.findItem(R.id.menu_start_scan).setVisible(false);
            menu.findItem(R.id.menu_refresh).setActionView(R.layout.actionbar_indeterminate_progress);
            menu.findItem(R.id.menu_refresh).setVisible(true);
        }

        refreshEmptyView();
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.menu_start_scan:
                /** Starting a discovery */
                startScanLeDevice();
                break;
            case R.id.menu_stop_scan:
                /** Stops the discovery */
                stopScanLeDevice();
                break;
        }
        return true;
    }

    @Override
    public void onResume() {
        super.onResume();

        // Ensures Bluetooth is enabled on the device.  If Bluetooth is not currently enabled,
        // fire an intent to display a dialog asking the user to grant permission to enable it.
        if (!mBluetoothAdapter.isEnabled()) {
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
        }

        /** Starting a discovery */
        startScanLeDevice();
    }

    @Override
    public void onPause() {
        super.onPause();
        stopScanLeDevice();
    }


    private void startScanLeDevice() {
        if (mScanner.isScanning())
            return; // Do nothing if already scanning

        mEntityAdapter.clear();
        boolean success = mScanner.startScanningFor(0);
        if (success) {
            invalidateOptionsMenu();
        } else {
            Toast.makeText(this, getString(R.string.error_scanning), Toast.LENGTH_LONG).show();
        }
    }

    /** *******************************************************************************************
     *
     *  Here, we stop the scanner...
     *
     *  ******************************************************************************************* */
    private void stopScanLeDevice() {
        mScanner.stopScanning();
        refreshEmptyView();
    }

    /*
      Looks for all previous bonded devices
     */
    private void getAllPreviousBondedDevice() {
        Log.d(TAG, "getAllPreviousBondedDevice() > Retrieving the list of bonded devices...");
        Set<BluetoothDevice> bondedDevices = mBluetoothAdapter.getBondedDevices();

        if (bondedDevices != null) {
            Log.d(TAG, String.format(Locale.getDefault(), "getAllPreviousBondedDevice() > List's size: %d", bondedDevices.size()));

            for (BluetoothDevice bd : bondedDevices) {

                boolean isPrinter = false;
                String result = bd.getName();
                ParcelUuid[] uuids = bd.getUuids();
                if (uuids != null) {
                    for (ParcelUuid uuid : uuids) {
                        if (uuid.getUuid().equals(BT_SPP_UUID)) {
                            Log.d(TAG, String.format(Locale.getDefault(), "getAllPreviousBondedDevice() > device `%s` seems to be a printer...", result));
                            isPrinter = true;
                            break;
                        }
                    }
                }

                if (!isPrinter) {
                    Log.d(TAG, "getAllPreviousBondedDevice() > Deleting " + result + "...");
                    deleteDevice(bd);
                }
            }
        }
    }


    private void deleteDevice(BluetoothDevice device) {
        final String deviceName = device.getName();
        try {
            Log.d(TAG, String.format(Locale.getDefault(), "deleteDevice() > `%s` will be deleted NOW !", deviceName));
            Method m = device.getClass().getMethod("removeBond", (Class[]) null);
            m.invoke(device, (Object[]) null);
        } catch (Exception e) {
            Log.e(TAG, String.format(Locale.getDefault(), "deleteDevice() > Removing `%s` has been failed !", deviceName, e.getMessage()));
        }
    }


    /* ******************************************************************************************** */
    /*                                                                                              */
    /* Sop Discovery Listener Implementation                                                        */
    /*                                                                                              */
    /* ******************************************************************************************** */
    /** *******************************************************************************************
     *  This is the callback function of the Sop Discoverer when it has found some Soprod watches
     *  of the desired model.
     *
     *  And that's it for the scanner...
     *  ******************************************************************************************* */
    @Override
    public void onSpecificDeviceFound(@NonNull Scanner scanner, @NonNull BleDeviceDescriptor descriptor) {
        if(descriptor.rssi() > -65 && descriptor.rssi() != 0) // Zero is sometimes given by bonded devices
        {
            mEntityAdapter.addEntity(descriptor);
            mEntityAdapter.notifyDataSetChanged();
            refreshEmptyView();
        }
        else
        {
            mEntityAdapter.removeEntity(descriptor);
            mEntityAdapter.notifyDataSetChanged();
            refreshEmptyView();
        }
    }

    @Override
    public void onDeviceDiscovered(@NonNull Scanner scanner, @NonNull BleDeviceDescriptor descriptor) {

    }

    @Override
    public void onDiscoveryStopped(@NonNull Scanner scanner) {
        // The scanner has been stopped manually or after a timeout
        invalidateOptionsMenu();
    }

    @Override
    public void onDiscoveryError(@NonNull Scanner scanner, @NonNull BleScanException throwable) {
        // Do nothing else than displaying the error
        Toast.makeText(this, "Scanning error code " + throwable.getReason(), Toast.LENGTH_SHORT).show();
        // TODO: handle the error here
    }

    /* OnEntityClickListener */
    @Override
    public void onEntitySelected(DiscoveredEntity entity) {
        // The user has selected a discovered BLE device in the list.
        // Stop scanning if enabled and start the main activity to connect to the device
        stopScanLeDevice();

        // Launch the main activity with entity device to connect to as argument.
        startMainActivity(entity);
    }

    /**
     * Show an empty when no Soprod Watches are available.
     */
    private void refreshEmptyView() {
        final View list = findViewById(R.id.devices_list);
        final View empty = findViewById(R.id.empty_view);

        if (list == null || empty == null)
            return;

        if (mEntityAdapter.getItemCount() == 0) {
            list.setVisibility(View.GONE);
            empty.setVisibility(View.VISIBLE);
        } else {
            list.setVisibility(View.VISIBLE);
            empty.setVisibility(View.GONE);
        }
    }

    private void startMainActivity(DiscoveredEntity entity) {
        // Launch the main activity with entity device to connect to as argument.
        final Intent i = new Intent(ScannerActivity.this, MainActivity.class);
        i.putExtra(HfBleConstants.HF_EXTRA_INTENT_KEY_DESC, entity);
        startActivity(i);
    }

}
