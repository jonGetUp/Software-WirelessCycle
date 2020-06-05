/*
 * ***********************************************************************************************
 * Copyright (c) 2017, Soprod SA, HES-SO Valais-Wallis, HEI, Infotronics
 * All rights reserved.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 *  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 *  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
 *  WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *  **************************************************************************************************
 *
 */
package ch.hevs.lib.soprod.watchkit.implementation.ble.services;

import android.Manifest;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.support.annotation.NonNull;
import android.support.annotation.RequiresPermission;
import android.util.Log;

import java.util.Locale;
import java.util.UUID;

import ch.hevs.ble.lib.core.BleService;
import ch.hevs.ble.lib.core.BleServiceObserver;
import ch.hevs.ble.lib.events.commands.BleCommand;
import ch.hevs.ble.lib.events.commands.CommandStatus;
import ch.hevs.ble.lib.events.responses.BleGattResponse;

/**
 *
 * @author Patrice Rudaz (patrice.rudaz@hevs.ch)
 * @version 1.0.1 - 2017/07/28
 */
public abstract class DefaultService implements BleServiceObserver {
    private final static String TAG = "DefaultService";

    protected final int WRITE_REQUEST = BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT;
    protected final int WRITE_COMMAND = BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE;

    /**
     * GATT error code returned by the Soprod Watch if the link is not secured when writing a value
     * to a characteristic handle or its CCCD handle.
     */
    protected final static int BLE_GATT_STATUS_ATTERR_INSUF_AUTHORIZATION = 0x08;

    /** BLE service */
    protected BluetoothGattService mService = null;

    /** Shared BLE service from the library */
    protected BleService mBleLib = null;


    /* ****************************************************************************************** */
    /*                                                                                            */
    /*                                CONSTRUCTOR'S SECTION                                       */
    /*                                                                                            */
    /* ****************************************************************************************** */
    protected DefaultService(@NonNull BleService bleLibService) {
        mBleLib = bleLibService;
        if (mBleLib == null)
            return; //No service connected now

        // Register as an observer to get notified and received events from the BLE service
        mBleLib.registerObserver(this);
    }


    /* ******************************************************************************************** */
    /*                                                                                              */
    /* Protected Section                                                                            */
    /*                                                                                              */
    /* ******************************************************************************************** */
    /**
     * Looks for the characteristic specified by the given UUID in the BLE service reference and
     * returns it.
     *
     * @param service   Pointer to the BLE service
     * @param charUuid  Characteristic's UUID to llok for.
     *
     * @return The characteristic found or null.
     */
    protected BluetoothGattCharacteristic getCharacteristic(BluetoothGattService service, UUID charUuid) {
        BluetoothGattCharacteristic c = service.getCharacteristic(charUuid);
        if (c == null) {
            Log.w(TAG, String.format(Locale.getDefault(), "Characteristic %s not found!", charUuid.toString()));
        } else {
            Log.i(TAG, String.format(Locale.getDefault(), "Characteristic %s found.", charUuid.toString()));
        }
        return c;
    }

    /**
     * Invalidate the service
     */
    protected void invalidateService() {
        mService = null;
    }

    /**
     * TO clear the flags of a read request
     */
    protected abstract void clearRequestedFlags();

    /* ******************************************************************************************** */
    /*                                                                                              */
    /* Getter's Section                                                                             */
    /*                                                                                              */
    /* ******************************************************************************************** */
    public BluetoothGattService getService() {
        return mService;
    }


    /* ******************************************************************************************** */
    /*                                                                                              */
    /* Public Section                                                                               */
    /*                                                                                              */
    /* ******************************************************************************************** */
    /**
     * @return {@code true} if the BLE service is still valid (not null) or {@code false} otherwise.
     */
    public boolean isValid() {
        return mService != null;
    }

    /**
     * register the discovered service and extracts the characteristics.
     * @param service   The discovered BLE service.
     */
    public abstract void discoverService(@NonNull BluetoothGattService service);


    /* ******************************************************************************************** */
    /*                                                                                              */
    /* Override's Section                                                                           */
    /*                                                                                              */
    /* ******************************************************************************************** */
    @Override
    public void onGattResponse(@NonNull BluetoothDevice device, @NonNull BleGattResponse response) {
        // We'll do nothing here
    }

    @Override
    public void onCommandError(BluetoothDevice device, @NonNull BleCommand bleCommand, @NonNull CommandStatus status) {
        // We'll do nothing here
    }

    @Override
    @RequiresPermission(Manifest.permission.BLUETOOTH_ADMIN)
    public void onGattError(BluetoothDevice device, @NonNull BleGattResponse response, int status) {

        if (status == BLE_GATT_STATUS_ATTERR_INSUF_AUTHORIZATION) {
            Log.d(TAG, "Clear the request's flags because of BLE GATT Error: BLE_GATT_STATUS_GATT_ERR_INSUF_AUTHORIZATION");
            clearRequestedFlags();
        }

    }
    @Override
    public void onConnectionError(BluetoothDevice device, @NonNull ConnectionErrorState errorState) {
        // We'll do nothing here
    }

    @Override
    public void onConnectionStateChanged(BluetoothDevice device, @NonNull ConnectionState state) {
        // We'll do nothing here
    }

    @Override
    public void onAdapterStateChanged(@NonNull BleAdapterState bleAdapterState) {
        // We'll do nothing here
    }

    @Override
    public void onBondingStateChanged(@NonNull BluetoothDevice device, @NonNull BondingState bondingState) {
        // We'll do nothing here
    }
}
