package ch.hevs.heatflow;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.support.annotation.NonNull;
import android.os.Bundle;
import android.support.v7.widget.CardView;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.ToggleButton;

import java.nio.ByteBuffer;
import java.util.Locale;
import java.util.Timer;
import java.util.TimerTask;

import ch.hevs.ble.lib.core.BleServiceObserver;
import ch.hevs.ble.lib.utils.BleManager;
import ch.hevs.heatflow.ble.core.BleConnectionState;
import ch.hevs.heatflow.ble.core.BleDevice;
import ch.hevs.heatflow.ble.core.BleDeviceDescriptor;
import ch.hevs.heatflow.ble.core.BleDeviceListener;
import ch.hevs.heatflow.ble.services.hevs.mhfs.MhfsService;
import ch.hevs.heatflow.ble.utils.HfBleConstants;
import ch.hevs.heatflow.common.HfMath;

import static android.view.View.INVISIBLE;
import static android.view.View.VISIBLE;
import static java.util.Objects.requireNonNull;

public class MainActivity extends DefaultActivity implements BleDeviceListener {
    private final static String TAG = "MainActivity";

    protected static final int REQUEST_ENABLE_BT = 1;


    private double mLastT1;
    private double mLastT2;

    // Register the Bluetooth pairing receiver if any
    private BroadcastReceiver mPairingReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (BluetoothDevice.ACTION_PAIRING_REQUEST.equals(intent.getAction())) {
                Log.e(TAG, "android.bluetooth.device.action.PAIRING_REQUEST");
                final BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                try {
                    // Successfully pairing
                    device.setPairingConfirmation(true);

                    device.getClass().getMethod("cancelPairingUserInput").invoke(device);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }
    };

    // Shared Bluetooth BLE service bound by this Activity
    private BleDevice mHfDevice;

    private ToggleButton mBtnMeasure;

    private Handler mHandler = new Handler();

    private BleConnectionState mBleState = BleConnectionState.DISCONNECTED;

    // A TimerTask to connect to the watch after having restarting the Bluetooth adapter...
    private class bindBleServiceTask extends TimerTask {

        @Override
        public void run() {
            synchronized (this) {
                mHfDevice.bindBleService(getApplicationContext());
            }
        }
    }


    /* ******************************************************************************************** */
    /*                                                                                              */
    /* Android Lifecycle                                                                            */
    /*                                                                                              */
    /* ******************************************************************************************** */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Toolbar toolbar = findViewById(R.id.toolbar);
        if (toolbar != null) {
            setSupportActionBar(toolbar);
            requireNonNull(getSupportActionBar()).setDisplayShowTitleEnabled(false);
            requireNonNull(getSupportActionBar()).setDisplayHomeAsUpEnabled(false);
        }

        // Get the discovered device descriptor
        final Intent i = getIntent();
        final BleDeviceDescriptor desc = (BleDeviceDescriptor) i.getSerializableExtra(HfBleConstants.HF_EXTRA_INTENT_KEY_DESC);
        if (desc == null) {
            Log.e(TAG, "No BLE device to connect to.");
            finish();
            return;
        }

        // Assigning the listener of click, LongClick, ...
        mBtnMeasure = findViewById(R.id.btn_start_measure);
        mBtnMeasure.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (mBtnMeasure.isChecked())
                    mHfDevice.EnableMeasurement();
                else
                    mHfDevice.disableMeasurement();
            }
        });

        // Intent filter for the Bluetooth pairing dialog
        final IntentFilter filter = new IntentFilter();
        filter.addAction("android.bluetooth.device.action.PAIRING_REQUEST");

        mPairingReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                if (BluetoothDevice.ACTION_PAIRING_REQUEST.equals(intent.getAction())) {
                    Log.e(TAG, "android.bluetooth.device.action.PAIRING_REQUEST");
                    final BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                    try {
                        // Successfully pairing
                        device.setPairingConfirmation(true);

                        device.getClass().getMethod("cancelPairingUserInput").invoke(device);
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
            }
        };
        registerReceiver(mPairingReceiver, filter);

        // Set the BLE device information to connect to
        mHfDevice = new BleDevice();
        mHfDevice.setDescriptor(desc);
        mHfDevice.addListener(this);

        /* ****************************************************************************************
         * NEW with SopWatchKit lib v0.9.0
         ****************************************************************************************** */
        if (checkBleAvailability()) {
            mHfDevice.bindBleService(getApplicationContext());
        }
    }

    @Override
    public void onDestroy() {

        if (mHfDevice.isConnected()) {
            mHfDevice.disableMeasurement();
            mHfDevice.removeListeners();
            mHfDevice.disconnect();
        }

        // Unregister the Bluetooth pairing receiver if any
        if (mPairingReceiver != null) {
            unregisterReceiver(mPairingReceiver);
            mPairingReceiver = null;
        }

        super.onDestroy();
    }

    /*
     * Use this method to instantiate your menu, and add your items to it. You
     * should return true if you have added items to it and want the menu to be displayed.
     */
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate our menu from the resources by using the menu inflater.
        getMenuInflater().inflate(R.menu.menu_main_activity, menu);
        return true;
    }

    /*
     * This method is called when one of the menu items to selected. These items
     * can be on the Action Bar, the overflow menu, or the standard options menu. You
     * should return true if you handle the selection.
     */
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.menu_scan_again:
            case android.R.id.home:
                backToDiscovery();
                return true;
        }

        return super.onOptionsItemSelected(item);
    }

    // NEW with SopWatchKit lib v0.9.0
    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        // User chose not to enable Bluetooth.
        if (requestCode == REQUEST_ENABLE_BT && resultCode == Activity.RESULT_CANCELED) {
            finish();
            return;
        }
        super.onActivityResult(requestCode, resultCode, data);

        // NEW with SopWatchKit lib v0.9.0
        Timer connectTimer = new Timer();
        connectTimer.schedule(new bindBleServiceTask(), 7000);
    }

    /* ******************************************************************************************** */
    /*                                                                                              */
    /* Interface implementation Section                                                             */
    /*                                                                                              */
    /* ******************************************************************************************** */

    /* ******************************************************************************************** */
    /* SopWatchListener Interface Implementation                                                    */
    /* ******************************************************************************************** */
    @Override
    public void onBleServiceReady() {
        // Automatically connects to the device upon successful start-up initialization
        connectBleDevice();
    }

    @Override
    public void onBleAdapterStateChanged(@NonNull BleServiceObserver.BleAdapterState adapterState) {
        if (adapterState == BleServiceObserver.BleAdapterState.ADAPTER_STATE_OFF) {
            Log.d(TAG, "Bluetooth has been turned OFF... Application will stops !");
            finish();
        }
    }

    @Override
    public void onBleDeviceBusy(@NonNull BleDevice device) {
        invalidateOptionsMenu();
    }

    @Override
    public void onBleDeviceReady(@NonNull BleDevice device) {
        invalidateOptionsMenu();
        mBleState = BleConnectionState.CONNECTED_READY;

        // The device is now connected and ready. Disable the loading view.
        showLoading(false);
    }

    @Override
    public void onBleDeviceConnecting(@NonNull BleDevice device) {
        invalidateOptionsMenu();
        mBleState = BleConnectionState.CONNECTING;

        // The device is now connecting. Enable the loading view
        showLoading(true);
    }

    @Override
    public void onBleDeviceConnected(@NonNull BleDevice device) {
        invalidateOptionsMenu();
        mBleState = BleConnectionState.INITIALIZING;

        // The device is now connected. But we still leave enable the loading view
        showLoading(true);
    }

    @Override
    public void onBleDeviceConnectionError(@NonNull BleDevice device, BleConnectionState errorState) {
        invalidateOptionsMenu();
        Log.e(TAG, String.format(Locale.getDefault(), "Connection to device '%s' error. ErrorState: %s", device.getBleAddress(), errorState.toString()));
        mBleState = BleConnectionState.DISCONNECTED;

        // The device is now connected and ready. Disable the loading view.
        showLoading(false);

        backToDiscovery();
    }

    @Override
    public void onBleDeviceDisconnected(@NonNull BleDevice device, int statusCode) {
        invalidateOptionsMenu();
        mBleState = BleConnectionState.DISCONNECTED;

        // The device is now connected and ready. Disable the loading view.
        showLoading(false);

        // Go back to discovery
        backToDiscovery();
    }

    @Override
    public void onAdsValuesChanged(@NonNull BleDevice device, @NonNull MhfsService service, byte adsId, byte mux, byte pga, boolean isFloatValue, Integer value) {
        if (adsId == 1) {
            updateTemperature(mux == 0, isFloatValue, value);
        } else {
            updateHeatflow(isFloatValue, value);
        }

    }

    @Override
    public void onDeviceInformationChanged(@NonNull BleDevice device) {
        // Checks what's coming in...
        updateHfDeviceInfo();
    }

    @Override
    public void onBatteryLevelChanged(@NonNull BleDevice device, int batteryLevel) {
        updateBatteryLevel(batteryLevel);
    }


    /* ******************************************************************************************** */
    /*                                                                                              */
    /* Public Section                                                                               */
    /*                                                                                              */
    /* ******************************************************************************************** */


    /* ******************************************************************************************** */
    /*                                                                                              */
    /* Protected Section                                                                            */
    /*                                                                                              */
    /* ******************************************************************************************** */


    /* ******************************************************************************************** */
    /*                                                                                              */
    /* Private Sections                                                                             */
    /*                                                                                              */
    /* ******************************************************************************************** */

    /* ******************************************************************************************** */
    /* Connection and reconnection...                                                               */
    /* ******************************************************************************************** */

    /** *******************************************************************************************
     * Connection retry.
     * <p>
     * Except for the case of the DFU function, when the app looses the connection with the watch,
     * it tries to reconnect directly.
     *  ******************************************************************************************* */
    private void tryToReconnect()  {
        Log.d(TAG, String.format(Locale.getDefault(), "tryToReconnect() > ... Trying to reconnect to %s ...", mHfDevice.getBleAddress()));
        connect();
    }

    // NEW with SopWatchKit lib v0.9.0
    private boolean checkBleAvailability() {
        // Checks if BLE is enabled
        if (!BleManager.isBtEnabled(getApplicationContext())) {
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
            return false;
        }
        return true;
    }


    /* ********************************************************************************************
     * Connection to the Soprod watch.
     * <p>
     * The BLE parameters of the watch are defined in its {@code descriptor}, which contains the
     * name, the address...
     *  ******************************************************************************************* */
    public void connect() {
        if (!mHfDevice.bindBleService(getApplicationContext()))
            connectBleDevice();
    }

    // NEW with SopWatchKit lib v0.9.0
    private void connectBleDevice() {
        // Connection request with a default timeout
        mHfDevice.connect(HfBleConstants.HF_BLE_CONN_TIMEOUT_MS);
    }


    /* ******************************************************************************************** */
    /* Android Activity and Fragments                                                               */
    /* ******************************************************************************************** */
    // Close the current activity and go back to Discovery
    private void backToDiscovery() {
        // Clearing the shared preferences
        finish();
    }


    /* ******************************************************************************************** */
    /* UI Related                                                                                   */
    /* ******************************************************************************************** */
    // Makes the loading/error view visible on the top of the application content with a progress bar.
    private void showLoading(final boolean visible) {
        String  state = "...";
        boolean connected = false;

        switch(mBleState) {
            case CONNECTED_READY:
                connected = true;
                state = getResources().getString(R.string.ble_conn_ready);
                break;

            case CONNECTING:
                connected = false;
                state = getResources().getString(R.string.ble_conn_connecting);
                break;

            case INITIALIZING:
                connected = false;
                state = getResources().getString(R.string.ble_conn_initializing);
                break;

            default:
                connected = false;
                state = getResources().getString(R.string.ble_conn_disconnected);
                break;
        }

        updateConnectionState(state, connected);
    }

    // Update the device information on the main activity UI
    private void updateHfDeviceInfo() {
    }

    private void updateConnectionState(final String state, final boolean connected) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                ImageView iv        = findViewById(R.id.img_connection_state);
                TextView  tv        = findViewById(R.id.tv_connection_state);
                CardView  cv1       = findViewById(R.id.card_view_result_1);
                CardView  cv2       = findViewById(R.id.card_view_result_2);
                ToggleButton tgb    = findViewById(R.id.btn_start_measure);

                if (tv != null) {
                    tv.setText(state);
                }

                if (iv != null) {
                    tgb.setClickable(connected);
                    if (connected) {
                        iv.setImageResource(R.drawable.ble_connected);
                        cv1.setVisibility(VISIBLE);
                        cv2.setVisibility(VISIBLE);
                    } else {
                        iv.setImageResource(R.drawable.ble_disconnected);
                        cv1.setVisibility(INVISIBLE);
                        cv2.setVisibility(INVISIBLE);
                    }
                }
            }
        });
    }

    // Update the battery level on the main activity UI
    public void updateBatteryLevel(@NonNull final Integer value) {
        final int viewId = R.id.imgBattery;
        final int battId   = R.id.tvDeviceBattery;

        mHandler.post(new Runnable() {
            @Override
            public void run() {
                ImageView iv = findViewById(viewId);
                TextView  tv = findViewById(battId);

                if (tv != null) {
                    tv.setText(String.format(Locale.getDefault(), "%d %%", value));
                }

                if (iv != null) {
                    if (value > 90) {
                        iv.setImageResource(R.drawable.battery100);
                    }
                    else if (value > 70) {
                        iv.setImageResource(R.drawable.battery075);
                    }
                    else if (value > 55) {
                        iv.setImageResource(R.drawable.battery060);
                    }
                    else if (value > 40) {
                        iv.setImageResource(R.drawable.battery050);
                    }
                    else if (value > 20) {
                        iv.setImageResource(R.drawable.battery025);
                    }
                    else if (value > 10) {
                        iv.setImageResource(R.drawable.battery005);
                    }
                    else {
                        iv.setImageResource(R.drawable.battery000);
                    }
                }
            }
        });
    }

    // Update the content of any TextView selectable by its id
    private void updateTextView(final int viewId, final String value, final int textColor) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                // UI Thread here
                TextView tv = findViewById(viewId);
                if (tv != null) {
                    tv.setText(value);
                    tv.setTextColor( textColor);
                    tv.setVisibility(VISIBLE);
                }
            }
        });
    }

    private void updateTemperature(boolean isT1, boolean isFloatValue, Integer value) {
        if (!isFloatValue) {
            Log.e(TAG, "Result should be a float value, but it's not !");
            return;
        }

        byte arrTemp[]  = new byte[4];
        arrTemp[3]      = (byte) ((value      ) & 0xff);
        arrTemp[2]      = (byte) ((value >>  8) & 0xff);
        arrTemp[1]      = (byte) ((value >> 16) & 0xff);
        arrTemp[0]      = (byte) ((value >> 24) & 0xff);

        ByteBuffer bbTemp   = ByteBuffer.wrap(arrTemp);

        final TextView tvMv = findViewById(isT1 ? R.id.txt_ads_1_diff_0_1:R.id.txt_ads_1_diff_2_3);
        final TextView tv   = findViewById(isT1 ? R.id.txt_ads_1_t_1:R.id.txt_ads_1_t_2);
        final float mv      = bbTemp.getFloat();
        final double temp    = HfMath.TemperatureFromVoltage(mv);

        if (isT1) {
            mLastT1 = temp;
        } else {
            mLastT2 = temp;
        }

        mHandler.post(new Runnable() {
            @Override
            public void run() {
                tvMv.setText(String.format(Locale.getDefault(), "%6.3f [V]", mv));
                tv.setText(String.format(Locale.getDefault(), "%6.3f [\u00B0C]", temp));
            }
        });
    }

    private void updateHeatflow(boolean isFloatValue, float value) {
        if (isFloatValue) {
            Log.e(TAG, "Result should be a unsigned int value, but it's not !");
            return;
        }

        final TextView tvR  = findViewById(R.id.txt_ads_2_raw_value);
        final TextView tv   = findViewById(R.id.txt_ads_2_q_val);
        final float raw     = value;
        final double q      = HfMath.HeatflowFromTemperature(raw, mLastT1, mLastT2);

        mHandler.post(new Runnable() {
            @Override
            public void run() {
                tvR.setText(String.format(Locale.getDefault(), "%6.3f [V]", raw));
                tv.setText(String.format(Locale.getDefault(), "%6.3f [kW/m2]", q/1000));
            }
        });
    }

}
