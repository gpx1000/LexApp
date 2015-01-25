package com.elasmotech.givingthroughglass.lexapp;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.GestureDetector;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;

import com.google.android.glass.view.WindowUtils;
import com.google.android.glass.widget.CardScrollView;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.Mat;

/**
 * An {@link Activity} that shows a menu allowing a selection of beacon navigation or SSD, As these are modes both can be active at once.
 * <p/>
 * The main content view is composed of a one-card {@link CardScrollView} that plays the SSD audio
 * created from the video viewFinder and plays a audio positioned to denote what direction to travel in using
 * bluetooth beacons.  This is the list of commands it uses:
 *      - START_IMAGING -- turns on SSD
 *      - STOP_THIS -- turns off SSD (wish could use STOP_IMAGING but not official keyword)
 *      - GET_DIRECTIONS -- after saying this, state the destination and will navigate via beacons
 *      - STOP_DIRECTIONS -- beacon navigation turns off when at final destination or this command is issued.
 * @see <a href="http://www.seeingwithsound.com/hificode_OpenCV.cpp">Seeing With Sound</a>
 * uses OpenCV:
 * @see <a href="http://www.opencv.org">Open CV</a>
 * If your Glassware intends to intercept swipe gestures, you should set the content view directly
 * and use a {@link com.google.android.glass.touchpad.GestureDetector}.
 *
 * @see <a href="https://developers.google.com/glass/develop/gdk/touch">GDK Developer Guide</a>
 */
public class MainActivity extends Activity implements CameraBridgeViewBase.CvCameraViewListener2, GestureDetector.OnGestureListener {
    /**
     * Voice Menu {@link View}.
     */
    static {
        System.loadLibrary("LexAppNative");
    }
    public Boolean isSSDRunning;
    private BeaconFinder mBeaconFinder;
    private static final String TAG = "OpenCVViewFinder";
    private GestureDetector mGestureDetector;
    private CameraView mView;
    private native void generateSSDFromMat(long matPtr);
    private native void SSDStart();
    private native void SSDStop();

    private BaseLoaderCallback mLoaderCallback = new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(int status) {
            switch (status) {
                case LoaderCallbackInterface.SUCCESS: {
                    Log.i(TAG, "OpenCV loaded successfully");
                    mView.enableView();
                }
                break;
                default: {
                    super.onManagerConnected(status);
                }
                break;
            }
        }
    };

    @Override
    protected void onCreate(Bundle bundle) {
        super.onCreate(bundle);

        getWindow().requestFeature(WindowUtils.FEATURE_VOICE_COMMANDS);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        isSSDRunning = true;
        mGestureDetector = new GestureDetector(this, this);
        mView = new CameraView(this, CameraBridgeViewBase.CAMERA_ID_ANY);
        mView.setCvCameraViewListener(this);
        setContentView(mView);
        mBeaconFinder = new BeaconFinder();
        mBeaconFinder.SetupBlueTooth(getBaseContext());
        mBeaconFinder.StartScan();
    }

    @Override
    protected void onResume() {
        super.onResume();
        // Load default libopencv_java.so
        if (OpenCVLoader.initDebug()) {
            if (mLoaderCallback != null) {
                mLoaderCallback.onManagerConnected(LoaderCallbackInterface.SUCCESS);
            }
        }
    }

    @Override
    protected void onPause() {
        if (mView != null) {
            mView.disableView();
        }
        super.onPause();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();

        if (mView != null) {
            mView.disableView();
        }
    }

    @Override
    public boolean onGenericMotionEvent(MotionEvent event) {
        if (mGestureDetector != null) {
            mGestureDetector.onTouchEvent(event);
        }

        return true;
    }

    @Override
    public boolean onCreatePanelMenu(int featureId, Menu menu) {
        if (featureId == WindowUtils.FEATURE_VOICE_COMMANDS) {
            getMenuInflater().inflate(R.menu.voice_menu, menu);
            return true;
        }
        // Good practice to pass through, for options menu.
        return super.onCreatePanelMenu(featureId, menu);
    }

    @Override
    public boolean onPreparePanel(int featureId, View view, Menu menu) {
        if (featureId == WindowUtils.FEATURE_VOICE_COMMANDS) {
            return true;
        }
        // Good practice to pass through, for options menu.
        return super.onPreparePanel(featureId, view, menu);
    }

    @Override
    public boolean onMenuItemSelected(int featureId, MenuItem item) {
        if (featureId == WindowUtils.FEATURE_VOICE_COMMANDS) {
            switch (item.getItemId()) {
                case R.id.get_directions: mBeaconFinder.StartScan(); break;
                case R.id.start_ssd:  isSSDRunning = true;  break;
                case R.id.stop_directions: mBeaconFinder.StopScan(); break;
                case R.id.stop_ssd:  isSSDRunning = false;  break;
                default: return true;  // No change.
            }
            return true;
        }
        return super.onMenuItemSelected(featureId, item);
    }

    @Override
    public boolean onSingleTapUp(MotionEvent motionEvent) {
        return false;
    }

    @Override
    public void onShowPress(MotionEvent motionEvent) {

    }

    @Override
    public void onLongPress(MotionEvent motionEvent) {

    }

    @Override
    public boolean onScroll(MotionEvent motionEvent, MotionEvent motionEvent2, float v, float v2) {
        return false;
    }

    @Override
    public boolean onFling(MotionEvent motionEvent, MotionEvent motionEvent2, float velocityX, float velocityY) {
        if(velocityY > 0.0f) //swipping down.
        {
            super.onBackPressed();
        }
        return false;
    }

    @Override
    public boolean onDown(MotionEvent motionEvent) {
        return false;
    }

    public void onCameraViewStarted(int width, int height) {
        SSDStart();
    }

    public void onCameraViewStopped() {
        SSDStop();
    }

    public Mat onCameraFrame(CameraBridgeViewBase.CvCameraViewFrame inputFrame) {
        Mat frame = inputFrame.rgba();
        if(isSSDRunning)
            generateSSDFromMat(frame.getNativeObjAddr());
        return frame;
    }
}
