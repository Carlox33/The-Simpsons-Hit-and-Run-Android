package org.libsdl.app;


import android.content.Context;
import android.content.pm.ActivityInfo;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Build;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Display;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.nio.charset.StandardCharsets;

/**
    SDLSurface. This is what we draw on, so we need to know when it's created
    in order to do anything useful.

    Because of this, that's where we set up the SDL thread
*/
public class SDLSurface extends SurfaceView implements SurfaceHolder.Callback,
    View.OnKeyListener, View.OnTouchListener, SensorEventListener  {

    // Sensors
    protected SensorManager mSensorManager;
    protected Display mDisplay;

    // Keep track of the surface size to normalize touch events
    protected float mWidth, mHeight;

    // Internal render resolution used by the Android Surface buffer
    private int mRenderWidth = 0;
    private int mRenderHeight = 0;

    // Avoid repeatedly applying the same fixed Surface size
    private int mLastFixedSurfaceWidth = 0;
    private int mLastFixedSurfaceHeight = 0;

    // Avoid repeatedly sending the same render resolution to native code
    private int mLastNativeSentRenderWidth = 0;
    private int mLastNativeSentRenderHeight = 0;

    // Avoid noisy resolution logs
    private int mLastLoggedRenderWidth = 0;
    private int mLastLoggedRenderHeight = 0;
    private int mLastLoggedDeviceWidth = 0;
    private int mLastLoggedDeviceHeight = 0;
    private int mLastLoggedSurfaceWidth = 0;
    private int mLastLoggedSurfaceHeight = 0;
    private int mLastLoggedTouchWidth = 0;
    private int mLastLoggedTouchHeight = 0;
    private boolean mNativeResolutionHookWarned = false;

    /*
     * Simpsons Hit & Run Android adaptive resolution config.
     *
     * Java is the source of truth for the internal render buffer size.
     * C++/PDDI will receive this size and use it as winWidth/winHeight.
     */
    private static final boolean SHAR_USE_ADAPTIVE_RENDER_RESOLUTION = true;

    // First safe profile. Later we can make this configurable through a text file.
    private static final String SHAR_RESOLUTION_CONFIG_FILE = "Simpsons_resolution.txt";

    private static final int SHAR_DEFAULT_TARGET_RENDER_HEIGHT = 1080;
    private static final int SHAR_MIN_TARGET_RENDER_HEIGHT = 300;

    /*
     * Fallback only. The real maximum is calculated at runtime using the
     * physical short side of the device.
     *
     * Example:
     * 2772x1280 -> max target height = 1280
     * 2400x1080 -> max target height = 1080
     */
    private static final int SHAR_FALLBACK_MAX_TARGET_RENDER_HEIGHT = 1080;

    // Runtime value loaded from Simpsons_resolution.txt.
    private int mSHARTargetRenderHeight = SHAR_DEFAULT_TARGET_RENDER_HEIGHT;

    // Round render width to avoid odd framebuffer sizes.
    private static final int SHAR_RENDER_WIDTH_ALIGNMENT = 8;

    // Keep logs useful but not invasive.
    private static final boolean SHAR_LOG_RESOLUTION = true;

    // ANDROID studio me da warning aqui pero la funcion está implementada correctamente luego en el gldisplay.cpp
    // y compila correctamente
    private static native void nativeSetSHARRenderResolution(int width, int height);

    // Is SurfaceView ready for rendering
    public boolean mIsSurfaceReady;

    // Startup
    public SDLSurface(Context context) {
        super(context);
        getHolder().addCallback(this);

        setFocusable(true);
        setFocusableInTouchMode(true);
        requestFocus();
        setOnKeyListener(this);
        setOnTouchListener(this);

        mDisplay = ((WindowManager)context.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();
        mSensorManager = (SensorManager)context.getSystemService(Context.SENSOR_SERVICE);

        /*
         * Simpsons Android:
         * Load/create the user resolution config before applying the Surface size.
         *
         * The config file is read before native code starts, so PDDI receives
         * the correct render resolution from the beginning.
         */
        loadSHARResolutionConfig();

        /*
         * Simpsons Android:
         * Apply the internal render resolution before the native surface starts.
         */
        applyAdaptiveFixedSurfaceSize("constructor", false);

        setOnGenericMotionListener(SDLActivity.getMotionListener());

        // Some arbitrary defaults to avoid a potential division by zero
        mWidth = 1.0f;
        mHeight = 1.0f;

        mIsSurfaceReady = false;
    }

// FUnciones para lectura del archivo de resoluciones para el usuario final
private File getSHARResolutionConfigFile() {
    File baseDir = null;

    try {
        baseDir = getContext().getExternalFilesDir(null);
    } catch (Exception ignored) {
    }

    if (baseDir == null) {
        try {
            baseDir = getContext().getFilesDir();
        } catch (Exception ignored) {
        }
    }

    if (baseDir == null) {
        return null;
    }

    return new File(baseDir, SHAR_RESOLUTION_CONFIG_FILE);
}

    private void createDefaultSHARResolutionConfigIfNeeded(File configFile) {
        if (configFile == null || configFile.exists()) {
            return;
        }

        try {
            File parent = configFile.getParentFile();

            if (parent != null && !parent.exists()) {
                parent.mkdirs();
            }

            OutputStreamWriter writer = new OutputStreamWriter(
                    new FileOutputStream(configFile),
                    StandardCharsets.UTF_8
            );

            writer.write("# Simpsons Hit & Run Android adaptive render resolution\n");
            writer.write("#\n");
            writer.write("# This file controls the internal render resolution used by the game on Android.\n");
            writer.write("# The game does not need to render at the full physical screen resolution.\n");
            writer.write("# Instead, the game renders at a lower internal resolution and Android scales the final image to fullscreen.\n");
            writer.write("#\n");
            writer.write("# You only need to change target_height.\n");
            writer.write("# The render width is calculated automatically to preserve the device aspect ratio.\n");
            writer.write("#\n");
            writer.write("# Standard values: 720, 800, 850, 900, 1080.\n");
            writer.write("# Recommended value: 720 or higher is an light and optimized game.\n");
            writer.write("# Higher values improve sharpness but increase GPU load.\n");
            writer.write("# Lower values improve performance but reduce image quality.\n");
            writer.write("# The maximum target_height is the device landscape height, which is the physical short side.\n");
            writer.write("# Example: on a 2772x1280 device, the maximum target_height is 1280.\n");
            writer.write("# If target_height is set above 1280 on that device, it will be clamped to 1280.\n");
            writer.write("# Values lower than 300 will be clamped to 300 automatically.\n");
            writer.write("#\n");
            writer.write("# How it works:\n");
            writer.write("# 1. The phone physical resolution is converted to landscape terms.\n");
            writer.write("# 2. target_height becomes the internal render height.\n");
            writer.write("# 3. The internal render width is calculated using the physical aspect ratio.\n");
            writer.write("# 4. The calculated width is rounded to a multiple of 8 to avoid odd framebuffer sizes.\n");
            writer.write("# 5. Android scales the final image to the full physical screen.\n");
            writer.write("#\n");
            writer.write("# Example with a POCO F7 with 2772x1280 device and target_height=1080:\n");
            writer.write("# physical landscape size = 2772x1280\n");
            writer.write("# target render height = 1080\n");
            writer.write("# raw render width = 1080 * 2772 / 1280 = 2338.875\n");
            writer.write("# rounded raw width = 2339\n");
            writer.write("# aligned width to multiple of 8 = 2336\n");
            writer.write("# final internal render resolution = 2336x1080\n");
            writer.write("# final visual output = scaled by Android to 2772x1280\n");
            writer.write("#\n");
            writer.write("# Examples:\n");
            writer.write("# target_height=720\n");
            writer.write("# target_height=900\n");
            writer.write("# target_height=1080\n");
            writer.write("# You can also use MAX to render at the maximum useful height for your device.\n");
            writer.write("# Example: on a 2772x1280 device, target_height=MAX means target_height=1280.\n");
            writer.write("\n");
            writer.write("target_height=" + SHAR_DEFAULT_TARGET_RENDER_HEIGHT + "\n");

            writer.flush();
            writer.close();

            if (SHAR_LOG_RESOLUTION) {
                Log.v(
                        "SDL",
                        "SHAR resolution config created: " + configFile.getAbsolutePath()
                );
            }
        } catch (Exception e) {
            if (SHAR_LOG_RESOLUTION) {
                Log.v(
                        "SDL",
                        "SHAR resolution config creation failed: " + e.getMessage()
                );
            }
        }
    }

    private void loadSHARResolutionConfig() {
        mSHARTargetRenderHeight = sanitizeSHARTargetRenderHeight(
                SHAR_DEFAULT_TARGET_RENDER_HEIGHT
        );

        File configFile = getSHARResolutionConfigFile();

        if (configFile == null) {
            if (SHAR_LOG_RESOLUTION) {
                Log.v(
                        "SDL",
                        "SHAR resolution config skipped: no valid config directory"
                );
            }
            return;
        }

        createDefaultSHARResolutionConfigIfNeeded(configFile);

        if (!configFile.exists()) {
            if (SHAR_LOG_RESOLUTION) {
                Log.v(
                        "SDL",
                        "SHAR resolution config missing, using default targetHeight=" +
                                mSHARTargetRenderHeight
                );
            }
            return;
        }

        try {
            BufferedReader reader = new BufferedReader(
                    new InputStreamReader(
                            new FileInputStream(configFile),
                            StandardCharsets.UTF_8
                    )
            );

            String line;

            while ((line = reader.readLine()) != null) {
                line = line.trim();

                if (line.length() == 0 || line.startsWith("#")) {
                    continue;
                }

                /*
                 * Supported formats:
                 *
                 * target_height=1080
                 * target_height = 1080
                 * 1080
                 */
                String valueText = line;

                if (line.startsWith("target_height")) {
                    int equalIndex = line.indexOf('=');

                    if (equalIndex < 0) {
                        continue;
                    }

                    valueText = line.substring(equalIndex + 1).trim();
                }

                int parsedValue;

                if (valueText.equalsIgnoreCase("max")) {
                    parsedValue = getDeviceMaxTargetRenderHeight();
                } else {
                    parsedValue = Integer.parseInt(valueText);
                }

                mSHARTargetRenderHeight = sanitizeSHARTargetRenderHeight(parsedValue);

                if (SHAR_LOG_RESOLUTION) {
                    Log.v(
                            "SDL",
                            "SHAR resolution config loaded: targetHeight=" +
                                    mSHARTargetRenderHeight +
                                    " maxDeviceTargetHeight=" +
                                    getDeviceMaxTargetRenderHeight() +
                                    " file=" +
                                    configFile.getAbsolutePath()
                    );
                }

                reader.close();
                return;
            }

            reader.close();

            if (SHAR_LOG_RESOLUTION) {
                Log.v(
                        "SDL",
                        "SHAR resolution config has no valid value, using default targetHeight=" +
                                mSHARTargetRenderHeight
                );
            }
        } catch (Exception e) {
            mSHARTargetRenderHeight = sanitizeSHARTargetRenderHeight(
                    SHAR_DEFAULT_TARGET_RENDER_HEIGHT
            );

            if (SHAR_LOG_RESOLUTION) {
                Log.v(
                        "SDL",
                        "SHAR resolution config read failed, using default targetHeight=" +
                                mSHARTargetRenderHeight +
                                " error=" +
                                e.getMessage()
                );
            }
        }
    }


    private int[] getPhysicalDisplaySize() {
        int[] size = new int[] { 0, 0 };

        try {
            DisplayMetrics realMetrics = new DisplayMetrics();

            if (Build.VERSION.SDK_INT >= 17) {
                mDisplay.getRealMetrics(realMetrics);
            } else {
                mDisplay.getMetrics(realMetrics);
            }

            size[0] = realMetrics.widthPixels;
            size[1] = realMetrics.heightPixels;
        } catch (Exception ignored) {
        }

        return size;
    }

    private int getDeviceMaxTargetRenderHeight() {
        int[] size = getPhysicalDisplaySize();

        int deviceWidth = size[0];
        int deviceHeight = size[1];

        if (deviceWidth <= 0 || deviceHeight <= 0) {
            return SHAR_FALLBACK_MAX_TARGET_RENDER_HEIGHT;
        }

        /*
         * Simpsons Android renders in landscape terms.
         * The maximum useful target render height is the physical short side.
         *
         * Example:
         * 2772x1280 -> max target height = 1280
         */
        return Math.min(deviceWidth, deviceHeight);
    }

    private int sanitizeSHARTargetRenderHeight(int value) {
        final int maxTargetHeight = getDeviceMaxTargetRenderHeight();

      /*
      *    Si es menor que el minimo, se establece al minimo
       */
        final int safeMinTargetHeight = Math.min(
                SHAR_MIN_TARGET_RENDER_HEIGHT,
                maxTargetHeight
        );

        if (value < SHAR_MIN_TARGET_RENDER_HEIGHT) {
            return safeMinTargetHeight;
        }

        if (value > maxTargetHeight) {
            return maxTargetHeight;
        }

        return value;
    }

    private static int roundToMultiple(int value, int multiple) {
        if (multiple <= 1) {
            return value;
        }

        return Math.round(value / (float) multiple) * multiple;
    }

    private static float clamp01(float value) {
        if (value < 0.0f) {
            return 0.0f;
        }

        if (value > 1.0f) {
            return 1.0f;
        }

        return value;
    }

    private float normalizeTouchX(MotionEvent event, int pointerIndex) {
        if (mWidth <= 0.0f) {
            return 0.0f;
        }

        return clamp01(event.getX(pointerIndex) / mWidth);
    }

    private float normalizeTouchY(MotionEvent event, int pointerIndex) {
        if (mHeight <= 0.0f) {
            return 0.0f;
        }

        return clamp01(event.getY(pointerIndex) / mHeight);
    }

    private void sendSHARRenderResolutionToNative(int width, int height, String reason) {
        if (width <= 0 || height <= 0) {
            return;
        }

        if (width == mLastNativeSentRenderWidth &&
                height == mLastNativeSentRenderHeight) {
            return;
        }

        try {
            nativeSetSHARRenderResolution(width, height);

            mLastNativeSentRenderWidth = width;
            mLastNativeSentRenderHeight = height;

            if (SHAR_LOG_RESOLUTION) {
                Log.v(
                        "SDL",
                        "SHAR native render resolution sent [" + reason + "]: " +
                                width + "x" + height
                );
            }
        } catch (UnsatisfiedLinkError e) {
            /*
             * Temporary safety while the native side is being added.
             * Once gldisplay.cpp implements nativeSetSHARRenderResolution(),
             * this warning should disappear.
             */
            if (!mNativeResolutionHookWarned) {
                mNativeResolutionHookWarned = true;

                Log.v(
                        "SDL",
                        "SHAR native render resolution hook not available yet: " +
                                e.getMessage()
                );
            }
        }
    }

    private void applySurfaceViewUpscale(int renderWidth, int renderHeight) {
        if (!SHAR_USE_ADAPTIVE_RENDER_RESOLUTION) {
            setScaleX(1.0f);
            setScaleY(1.0f);
            return;
        }

        if (renderWidth <= 0 || renderHeight <= 0) {
            return;
        }

        int[] size = getPhysicalDisplaySize();

        int deviceWidth = size[0];
        int deviceHeight = size[1];

        if (deviceWidth <= 0 || deviceHeight <= 0) {
            return;
        }

        /*
         * Simpsons Android runs in landscape.
         * Upscale the fixed internal render buffer to the physical landscape size.
         */
        int targetViewWidth = Math.max(deviceWidth, deviceHeight);
        int targetViewHeight = Math.min(deviceWidth, deviceHeight);

        final float scaleX = targetViewWidth / (float) renderWidth;
        final float scaleY = targetViewHeight / (float) renderHeight;

        /*
         * Apply after layout traversal, because SurfaceView/Android can override
         * transforms during surfaceChanged/onConfigurationChanged/resume.
         */
        post(new Runnable() {
            @Override
            public void run() {
                setPivotX(0.0f);
                setPivotY(0.0f);
                setTranslationX(0.0f);
                setTranslationY(0.0f);
                setScaleX(scaleX);
                setScaleY(scaleY);
            }
        });
    }

    private void applyAdaptiveFixedSurfaceSize(String reason, boolean force) {
        if (!SHAR_USE_ADAPTIVE_RENDER_RESOLUTION) {
            setScaleX(1.0f);
            setScaleY(1.0f);
            return;
        }

        int[] size = getPhysicalDisplaySize();

        int deviceWidth = size[0];
        int deviceHeight = size[1];

        if (deviceWidth <= 0 || deviceHeight <= 0) {
            if (SHAR_LOG_RESOLUTION) {
                Log.v("SDL", "SHAR adaptive resolution skipped [" + reason + "]: invalid device size");
            }
            return;
        }

        /*
         * Calculate the render resolution in landscape terms:
         *
         * physical screen: longSide x shortSide
         * render target: adaptiveWidth x SHAR_TARGET_RENDER_HEIGHT
         *
         * Example:
         * 2772x1280
         * targetHeight = 720
         * targetWidth = 720 * 2772 / 1280 = 1559.25 -> 1560
         */
        int longSide = Math.max(deviceWidth, deviceHeight);
        int shortSide = Math.min(deviceWidth, deviceHeight);

        int targetHeight = sanitizeSHARTargetRenderHeight(mSHARTargetRenderHeight);
        mSHARTargetRenderHeight = targetHeight;
        int targetWidth = (int) Math.round(targetHeight * (longSide / (double) shortSide));

        targetWidth = roundToMultiple(targetWidth, SHAR_RENDER_WIDTH_ALIGNMENT);

        if (targetWidth <= 0 || targetHeight <= 0) {
            if (SHAR_LOG_RESOLUTION) {
                Log.v("SDL", "SHAR adaptive resolution skipped [" + reason + "]: invalid target size");
            }
            return;
        }

        final boolean changed =
                targetWidth != mLastFixedSurfaceWidth ||
                        targetHeight != mLastFixedSurfaceHeight;

        if (!changed && !force) {
            return;
        }

        mRenderWidth = targetWidth;
        mRenderHeight = targetHeight;

        mLastFixedSurfaceWidth = targetWidth;
        mLastFixedSurfaceHeight = targetHeight;

        /*
         * Send the selected adaptive render resolution to the native game layer.
         * gldisplay.cpp will later use this as pglDisplay winWidth/winHeight.
         */
        sendSHARRenderResolutionToNative(mRenderWidth, mRenderHeight, reason);

        /*
         * This is the key Android-side performance change:
         * reduce the real Surface buffer size.
         */
        getHolder().setFixedSize(mRenderWidth, mRenderHeight);

        /*
         * Some Android devices do not visually upscale the SurfaceView automatically
         * after setFixedSize(), so we explicitly scale it to fullscreen.
         */
        applySurfaceViewUpscale(mRenderWidth, mRenderHeight);

        logAdaptiveResolutionIfNeeded(
                reason,
                longSide,
                shortSide,
                mRenderWidth,
                mRenderHeight,
                changed
        );
    }


    private void logAdaptiveResolutionIfNeeded(
            String reason,
            int deviceLandscapeWidth,
            int deviceLandscapeHeight,
            int renderWidth,
            int renderHeight,
            boolean changed
    ) {
        if (!SHAR_LOG_RESOLUTION) {
            return;
        }

        /*
         * Log only when:
         * - resolution changed,
         * - first startup,
         * - app resumes.
         *
         * Avoid logging repeatedly from surfaceCreated if values are unchanged.
         */
        boolean firstLog =
                mLastLoggedRenderWidth <= 0 ||
                        mLastLoggedRenderHeight <= 0;

        boolean shouldLog =
                changed ||
                        firstLog ||
                        "handleResume".equals(reason);

        if (!shouldLog) {
            return;
        }

        mLastLoggedRenderWidth = renderWidth;
        mLastLoggedRenderHeight = renderHeight;
        mLastLoggedDeviceWidth = deviceLandscapeWidth;
        mLastLoggedDeviceHeight = deviceLandscapeHeight;

        float scaleX = deviceLandscapeWidth / (float) renderWidth;
        float scaleY = deviceLandscapeHeight / (float) renderHeight;

        Log.v(
                "SDL",
                "SHAR adaptive resolution [" + reason + "]: " +
                        "device=" + deviceLandscapeWidth + "x" + deviceLandscapeHeight +
                        " render=" + renderWidth + "x" + renderHeight +
                        " targetHeight=" + mSHARTargetRenderHeight +
                        " scale=" + scaleX + "x" + scaleY
        );
    }

    private void logSurfaceSizesIfChanged(
            String reason,
            int surfaceWidth,
            int surfaceHeight,
            int touchWidth,
            int touchHeight,
            int deviceWidth,
            int deviceHeight
    ) {
        if (!SHAR_LOG_RESOLUTION) {
            return;
        }

        boolean changed =
                surfaceWidth != mLastLoggedSurfaceWidth ||
                        surfaceHeight != mLastLoggedSurfaceHeight ||
                        touchWidth != mLastLoggedTouchWidth ||
                        touchHeight != mLastLoggedTouchHeight ||
                        deviceWidth != mLastLoggedDeviceWidth ||
                        deviceHeight != mLastLoggedDeviceHeight;

        if (!changed) {
            return;
        }

        mLastLoggedSurfaceWidth = surfaceWidth;
        mLastLoggedSurfaceHeight = surfaceHeight;
        mLastLoggedTouchWidth = touchWidth;
        mLastLoggedTouchHeight = touchHeight;
        mLastLoggedDeviceWidth = deviceWidth;
        mLastLoggedDeviceHeight = deviceHeight;

        Log.v(
                "SDL",
                "SHAR surface sizes [" + reason + "]: " +
                        "surface=" + surfaceWidth + "x" + surfaceHeight +
                        " touch=" + touchWidth + "x" + touchHeight +
                        " device=" + deviceWidth + "x" + deviceHeight
        );
    }

    public void handlePause() {
        enableSensor(Sensor.TYPE_ACCELEROMETER, false);
    }

    public void handleResume() {
        setFocusable(true);
        setFocusableInTouchMode(true);
        requestFocus();
        setOnKeyListener(this);
        setOnTouchListener(this);

        /*
         * Re-apply the fixed Surface size after resume in case Android recreated
         * the Surface or display metrics changed.
         */
        applyAdaptiveFixedSurfaceSize("handleResume", true);

        enableSensor(Sensor.TYPE_ACCELEROMETER, true);
    }

    public Surface getNativeSurface() {
        return getHolder().getSurface();
    }

    // Called when we have a valid drawing surface
    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        Log.v("SDL", "surfaceCreated()");
        /*
         * Ensure the Surface buffer size is applied before notifying native code.
         */
        applyAdaptiveFixedSurfaceSize("surfaceCreated", true);

        SDLActivity.onNativeSurfaceCreated();
    }

    // Called when we lose the surface
    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        Log.v("SDL", "surfaceDestroyed()");

        // Transition to pause, if needed
        SDLActivity.mNextNativeState = SDLActivity.NativeState.PAUSED;
        SDLActivity.handleNativeState();

        mIsSurfaceReady = false;
        SDLActivity.onNativeSurfaceDestroyed();
    }

    // Called when the surface is resized
    @Override
    public void surfaceChanged(SurfaceHolder holder,
                               int format, int width, int height) {
        Log.v("SDL", "surfaceChanged()");

        if (SDLActivity.mSingleton == null) {
            return;
        }

        int nDeviceWidth = width;
        int nDeviceHeight = height;

        try {
            if (Build.VERSION.SDK_INT >= 17 /* Android 4.2 (JELLY_BEAN_MR1) */) {
                DisplayMetrics realMetrics = new DisplayMetrics();
                mDisplay.getRealMetrics(realMetrics);
                nDeviceWidth = realMetrics.widthPixels;
                nDeviceHeight = realMetrics.heightPixels;
            }
        } catch(Exception ignored) {
        }

        /*
         * width/height are the actual Surface buffer size.
         * With getHolder().setFixedSize(), these should normally match the adaptive
         * render resolution, for example 1560x720.
         */
        mRenderWidth = width;
        mRenderHeight = height;

        /*
         * Send the confirmed Surface buffer size to the native display layer.
         * This call is cheap because sendSHARRenderResolutionToNative() ignores
         * duplicated width/height values.
         */
        sendSHARRenderResolutionToNative(mRenderWidth, mRenderHeight, "surfaceChanged");

        /*
         * Apply visual fullscreen scaling after Android reports the final Surface size.
         * This keeps the reduced internal buffer stretched to the physical display.
         */
        applySurfaceViewUpscale(mRenderWidth, mRenderHeight);

        /*
         * mWidth/mHeight are used only to normalize touch coordinates.
         * Touch events are based on the visible SurfaceView area, not necessarily
         * the internal render buffer size.
         */
        int touchWidth = getWidth();
        int touchHeight = getHeight();

        if (touchWidth <= 0 || touchHeight <= 0) {
            touchWidth = nDeviceWidth;
            touchHeight = nDeviceHeight;
        }

        mWidth = touchWidth;
        mHeight = touchHeight;

        logSurfaceSizesIfChanged(
                "surfaceChanged",
                width,
                height,
                touchWidth,
                touchHeight,
                nDeviceWidth,
                nDeviceHeight
        );

        synchronized(SDLActivity.getContext()) {
            // In case we're waiting on a size change after going fullscreen, send a notification.
            SDLActivity.getContext().notifyAll();
        }

        /*
         * Keep the standard SDL path informed.
         *
         * surfaceWidth/surfaceHeight = internal Surface buffer size.
         * deviceWidth/deviceHeight   = physical device size.
         */
        SDLActivity.nativeSetScreenResolution(
                width,
                height,
                nDeviceWidth,
                nDeviceHeight,
                mDisplay.getRefreshRate()
        );

        SDLActivity.onNativeResize();

        // Prevent a screen distortion glitch,
        // for instance when the device is in Landscape and a Portrait App is resumed.
        boolean skip = false;
        int requestedOrientation = SDLActivity.mSingleton.getRequestedOrientation();

        if (requestedOrientation == ActivityInfo.SCREEN_ORIENTATION_PORTRAIT ||
                requestedOrientation == ActivityInfo.SCREEN_ORIENTATION_SENSOR_PORTRAIT) {
            if (mWidth > mHeight) {
                skip = true;
            }
        } else if (requestedOrientation == ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE ||
                requestedOrientation == ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE) {
            if (mWidth < mHeight) {
                skip = true;
            }
        }

        // Special Patch for Square Resolution: Black Berry Passport
        if (skip) {
            double min = Math.min(mWidth, mHeight);
            double max = Math.max(mWidth, mHeight);

            if (max / min < 1.20) {
                Log.v("SDL", "Don't skip on such aspect-ratio. Could be a square resolution.");
                skip = false;
            }
        }

        // Don't skip in MultiWindow.
        if (skip) {
            if (Build.VERSION.SDK_INT >= 24 /* Android 7.0 (N) */) {
                if (SDLActivity.mSingleton.isInMultiWindowMode()) {
                    Log.v("SDL", "Don't skip in Multi-Window");
                    skip = false;
                }
            }
        }

        if (skip) {
            Log.v("SDL", "Skip .. Surface is not ready.");
            mIsSurfaceReady = false;
            return;
        }

        /* If the surface has been previously destroyed by onNativeSurfaceDestroyed, recreate it here */
        SDLActivity.onNativeSurfaceChanged();

        /* Surface is ready */
        mIsSurfaceReady = true;

        SDLActivity.mNextNativeState = SDLActivity.NativeState.RESUMED;
        SDLActivity.handleNativeState();
    }

    // Key events
    @Override
    public boolean onKey(View v, int keyCode, KeyEvent event) {
        return SDLActivity.handleKeyEvent(v, keyCode, event, null);
    }

    // Touch events
    @Override
    public boolean onTouch(View v, MotionEvent event) {
        /* Ref: http://developer.android.com/training/gestures/multi.html */
        int touchDevId = event.getDeviceId();
        final int pointerCount = event.getPointerCount();
        int action = event.getActionMasked();
        int pointerFingerId;
        int i = -1;
        float x,y,p;

        /*
         * Prevent id to be -1, since it's used in SDL internal for synthetic events
         * Appears when using Android emulator, eg:
         *  adb shell input mouse tap 100 100
         *  adb shell input touchscreen tap 100 100
         */
        if (touchDevId < 0) {
            touchDevId -= 1;
        }

        // 12290 = Samsung DeX mode desktop mouse
        // 12290 = 0x3002 = 0x2002 | 0x1002 = SOURCE_MOUSE | SOURCE_TOUCHSCREEN
        // 0x2   = SOURCE_CLASS_POINTER
        if (event.getSource() == InputDevice.SOURCE_MOUSE || event.getSource() == (InputDevice.SOURCE_MOUSE | InputDevice.SOURCE_TOUCHSCREEN)) {
            int mouseButton = 1;
            try {
                Object object = event.getClass().getMethod("getButtonState").invoke(event);
                if (object != null) {
                    mouseButton = (Integer) object;
                }
            } catch(Exception ignored) {
            }

            // We need to check if we're in relative mouse mode and get the axis offset rather than the x/y values
            // if we are.  We'll leverage our existing mouse motion listener
            SDLGenericMotionListener_API12 motionListener = SDLActivity.getMotionListener();
            x = motionListener.getEventX(event);
            y = motionListener.getEventY(event);

            SDLActivity.onNativeMouse(mouseButton, action, x, y, motionListener.inRelativeMode());
        } else {
            switch(action) {
                case MotionEvent.ACTION_MOVE:
                    for (i = 0; i < pointerCount; i++) {
                        pointerFingerId = event.getPointerId(i);
                        x = normalizeTouchX(event, i);
                        y = normalizeTouchY(event, i);
                        p = event.getPressure(i);
                        if (p > 1.0f) {
                            // may be larger than 1.0f on some devices
                            // see the documentation of getPressure(i)
                            p = 1.0f;
                        }
                        SDLActivity.onNativeTouch(touchDevId, pointerFingerId, action, x, y, p);
                    }
                    break;

                case MotionEvent.ACTION_UP:
                case MotionEvent.ACTION_DOWN:
                    // Primary pointer up/down, the index is always zero
                    i = 0;
                    /* fallthrough */
                case MotionEvent.ACTION_POINTER_UP:
                case MotionEvent.ACTION_POINTER_DOWN:
                    // Non primary pointer up/down
                    if (i == -1) {
                        i = event.getActionIndex();
                    }

                    pointerFingerId = event.getPointerId(i);
                    x = normalizeTouchX(event, i);
                    y = normalizeTouchY(event, i);
                    p = event.getPressure(i);
                    if (p > 1.0f) {
                        // may be larger than 1.0f on some devices
                        // see the documentation of getPressure(i)
                        p = 1.0f;
                    }
                    SDLActivity.onNativeTouch(touchDevId, pointerFingerId, action, x, y, p);
                    break;

                case MotionEvent.ACTION_CANCEL:
                    for (i = 0; i < pointerCount; i++) {
                        pointerFingerId = event.getPointerId(i);
                        x = normalizeTouchX(event, i);
                        y = normalizeTouchY(event, i);
                        p = event.getPressure(i);
                        if (p > 1.0f) {
                            // may be larger than 1.0f on some devices
                            // see the documentation of getPressure(i)
                            p = 1.0f;
                        }
                        SDLActivity.onNativeTouch(touchDevId, pointerFingerId, MotionEvent.ACTION_UP, x, y, p);
                    }
                    break;

                default:
                    break;
            }
        }

        return true;
   }

    // Sensor events
    public void enableSensor(int sensortype, boolean enabled) {
        // TODO: This uses getDefaultSensor - what if we have >1 accels?
        if (enabled) {
            mSensorManager.registerListener(this,
                            mSensorManager.getDefaultSensor(sensortype),
                            SensorManager.SENSOR_DELAY_GAME, null);
        } else {
            mSensorManager.unregisterListener(this,
                            mSensorManager.getDefaultSensor(sensortype));
        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {
        // TODO
    }

    @Override
    public void onSensorChanged(SensorEvent event) {
        if (event.sensor.getType() == Sensor.TYPE_ACCELEROMETER) {

            // Since we may have an orientation set, we won't receive onConfigurationChanged events.
            // We thus should check here.
            int newOrientation;

            float x, y;
            switch (mDisplay.getRotation()) {
                case Surface.ROTATION_90:
                    x = -event.values[1];
                    y = event.values[0];
                    newOrientation = SDLActivity.SDL_ORIENTATION_LANDSCAPE;
                    break;
                case Surface.ROTATION_270:
                    x = event.values[1];
                    y = -event.values[0];
                    newOrientation = SDLActivity.SDL_ORIENTATION_LANDSCAPE_FLIPPED;
                    break;
                case Surface.ROTATION_180:
                    x = -event.values[0];
                    y = -event.values[1];
                    newOrientation = SDLActivity.SDL_ORIENTATION_PORTRAIT_FLIPPED;
                    break;
                case Surface.ROTATION_0:
                default:
                    x = event.values[0];
                    y = event.values[1];
                    newOrientation = SDLActivity.SDL_ORIENTATION_PORTRAIT;
                    break;
            }

            if (newOrientation != SDLActivity.mCurrentOrientation) {
                SDLActivity.mCurrentOrientation = newOrientation;
                SDLActivity.onNativeOrientationChanged(newOrientation);
            }

            SDLActivity.onNativeAccel(-x / SensorManager.GRAVITY_EARTH,
                                      y / SensorManager.GRAVITY_EARTH,
                                      event.values[2] / SensorManager.GRAVITY_EARTH);


        }
    }

    // Captured pointer events for API 26.
    public boolean onCapturedPointerEvent(MotionEvent event)
    {
        int action = event.getActionMasked();

        float x, y;
        switch (action) {
            case MotionEvent.ACTION_SCROLL:
                x = event.getAxisValue(MotionEvent.AXIS_HSCROLL, 0);
                y = event.getAxisValue(MotionEvent.AXIS_VSCROLL, 0);
                SDLActivity.onNativeMouse(0, action, x, y, false);
                return true;

            case MotionEvent.ACTION_HOVER_MOVE:
            case MotionEvent.ACTION_MOVE:
                x = event.getX(0);
                y = event.getY(0);
                SDLActivity.onNativeMouse(0, action, x, y, true);
                return true;

            case MotionEvent.ACTION_BUTTON_PRESS:
            case MotionEvent.ACTION_BUTTON_RELEASE:

                // Change our action value to what SDL's code expects.
                if (action == MotionEvent.ACTION_BUTTON_PRESS) {
                    action = MotionEvent.ACTION_DOWN;
                } else { /* MotionEvent.ACTION_BUTTON_RELEASE */
                    action = MotionEvent.ACTION_UP;
                }

                x = event.getX(0);
                y = event.getY(0);
                int button = event.getButtonState();

                SDLActivity.onNativeMouse(button, action, x, y, true);
                return true;
        }

        return false;
    }
}
