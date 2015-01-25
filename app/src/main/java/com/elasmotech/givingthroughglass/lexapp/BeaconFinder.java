package com.elasmotech.givingthroughglass.lexapp;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothManager;
import android.content.Context;
import android.os.Handler;

public class BeaconFinder {
    private BluetoothAdapter mBluetoothAdapter;
    private boolean mScanning;
    private Handler mHandler;
    // Stops scanning after 10 seconds.
    private static final long SCAN_PERIOD = 10000;

//    static {
//        System.loadLibrary("LexAppNative");
//    }

    public native static void addDevice(String deviceName, int rssi);

    // Device scan callback.
    private BluetoothAdapter.LeScanCallback mLeScanCallback =
            new BluetoothAdapter.LeScanCallback() {
                @Override
                public void onLeScan(final BluetoothDevice device, final int rssi, byte[] scanRecord) {
//                    addDevice(device.getName(), rssi);
                }
            };

    private void scanLeDevice(final boolean enable) {
        if (enable) {
            // Stops scanning after a pre-defined scan period.
            mHandler.postDelayed(new Runnable() {
                @Override
                public void run() {
                    mScanning = false;
                    mBluetoothAdapter.stopLeScan(mLeScanCallback);
                }
            }, SCAN_PERIOD);

            mScanning = true;
            mBluetoothAdapter.startLeScan(mLeScanCallback);
        } else {
            mScanning = false;
            mBluetoothAdapter.stopLeScan(mLeScanCallback);
        }
    }

    public void SetupBlueTooth(Context context)
    {
        mHandler = new Handler();
        // Initializes Bluetooth adapter.
        final BluetoothManager bluetoothManager =
                (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);
        mBluetoothAdapter = bluetoothManager.getAdapter();
    }

    public void StartScan()
    {
        if(!mScanning)
        {
            scanLeDevice(true);
        }
    }

    public void StopScan()
    {
        if(mScanning)
        {
            scanLeDevice(false);
        }
    }
}
