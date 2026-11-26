package com.c4rlox.simpsons;

import android.Manifest;
import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.content.res.ColorStateList;
import android.graphics.Color;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.provider.Settings;
import android.util.Log;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;

import org.libsdl.app.R;
import org.libsdl.app.SDLActivity;
import org.libsdl.app.SDLSurface;

import java.io.File;
import java.io.IOException;

public class SimpsonsActivity extends SDLActivity {
    private static final String TAG = "SimpsonsStorage";
    private static final String STORAGE_PREFS = "SimpsonsStorage";
    private static final String KEY_SHARED_STORAGE = "use_shared_game_storage";
    private static final String KEY_INITIAL_GAME_FILES_CONFIRMED = "initial_game_files_confirmed";
    private static final String KEY_STORAGE_SETUP_PENDING = "storage_setup_pending";
    private static final String KEY_STORAGE_CLEANUP_PENDING = "storage_cleanup_pending";
    private static final int REQUEST_STORAGE = 4201;

    // Permission chooses the candidate only during new-install setup.
    // Once selected, the mode stays fixed before the Surface/native startup.
    private boolean mUseSharedStorage;
    private boolean mStorageModeSelected;
    private boolean mAwaitingStorageChoice;
    private volatile boolean mStorageReady;
    private boolean mActivityResumed;
    // Written by the native drive after the normal loader opens the three basic
    // game files. This confirms file access, not a complete successful game boot.
    private volatile boolean mInitialGameFilesConfirmed;
    private ScrollView mStorageScreen;
    private TextView mStorageMessage;
    private Button mPermissionButton;
    private Button mExitButton;
    private String mStorageError;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        mInitialGameFilesConfirmed = getSharedPreferences(STORAGE_PREFS, MODE_PRIVATE)
                .getBoolean(KEY_INITIAL_GAME_FILES_CONFIRMED, false);
        // mStorageReady is still false: do not construct SDLSurface/read its files yet.
        super.onCreate(savedInstanceState);
    }

    private boolean selectStorageMode() {
        if (mStorageModeSelected) {
            return true;
        }
        SharedPreferences preferences = getSharedPreferences(STORAGE_PREFS, MODE_PRIVATE);
        if (preferences.contains(KEY_SHARED_STORAGE)) {
            mUseSharedStorage = preferences.getBoolean(KEY_SHARED_STORAGE, false);
        } else {
            // A partial old installation must not silently move to another folder.
            // Configuration/touch files alone are not evidence of PC game assets.
            File legacyRoot = getExternalFilesDir(null);
            if (legacyRoot == null) {
                // An unavailable volume is not evidence of a new installation.
                // Retry later rather than persisting a switch away from legacy.
                return false;
            }
            if (!preferences.getBoolean(KEY_STORAGE_SETUP_PENDING, false)
                    && hasLegacyGameAssets(legacyRoot)) {
                mUseSharedStorage = false;
                if (!preferences.edit().putBoolean(KEY_SHARED_STORAGE, false).commit()) {
                    mStorageError = getString(R.string.storage_preferences_error);
                    return false;
                }
            } else {
                mAwaitingStorageChoice = true;
                mUseSharedStorage = hasSharedStoragePermission();
                // Remember new-install setup across restarts: partially copied files
                // must not later be mistaken for an old legacy installation.
                if (!preferences.getBoolean(KEY_STORAGE_SETUP_PENDING, false)
                        && !preferences.edit().putBoolean(KEY_STORAGE_SETUP_PENDING, true).commit()) {
                    mStorageError = getString(R.string.storage_preferences_error);
                    return false;
                }
                if (!ensureStorageDirectory(legacyRoot)) {
                    return false;
                }
                if (mUseSharedStorage
                        && !ensureStorageDirectory(SHARStoragePaths.getSharedGameDirectory(this))) {
                    return false;
                }
                if (!hasInitialGameAssets(getActiveStorageDirectory())) {
                    mStorageError = getString(R.string.storage_files_pending);
                    return false;
                }
                // Lock the mode before constructing SDLSurface or starting native code.
                if (!preferences.edit()
                        .putBoolean(KEY_SHARED_STORAGE, mUseSharedStorage)
                        .putBoolean(KEY_INITIAL_GAME_FILES_CONFIRMED, false)
                        .putBoolean(KEY_STORAGE_CLEANUP_PENDING, true)
                        .remove(KEY_STORAGE_SETUP_PENDING).commit()) {
                    mStorageError = getString(R.string.storage_preferences_error);
                    return false;
                }
                mInitialGameFilesConfirmed = false;
            }
        }

        mAwaitingStorageChoice = false;
        mStorageModeSelected = true;
        return true;
    }

    @Override
    public boolean usesSharedGameStorage() {
        return mUseSharedStorage;
    }

    @Override
    protected boolean isNativeStartupReady() {
        return mStorageReady && (!mUseSharedStorage || hasSharedStoragePermission());
    }

    @Override
    protected SDLSurface createSDLSurface(Context context) {
        // Set the selected mode before any native reader resolves its storage root.
        nativeSetenv("SHAR_STORAGE_MODE", mUseSharedStorage ? "shared" : "legacy");
        nativeSetenv("SHAR_INITIAL_GAME_FILES_CONFIRMED", mInitialGameFilesConfirmed ? "1" : "0");
        return super.createSDLSurface(context);
    }

    // Called through JNI by sdldrive.cpp after the existing OpenFile operations
    // succeed for scripts.rcf, soundfx.rcf and art/frontend/scrooby/bootup.p3d.
    public synchronized boolean onInitialGameFilesOpened() {
        if (mInitialGameFilesConfirmed) {
            cleanupUnusedStorageDirectory();
            return true;
        }
        if (!mStorageReady) {
            return false;
        }

        // This runs on the drive thread, not the UI thread. Save once, synchronously,
        // so the flag is persisted before the native drive disables its tracking.
        if (!getSharedPreferences(STORAGE_PREFS, MODE_PRIVATE).edit()
                .putBoolean(KEY_INITIAL_GAME_FILES_CONFIRMED, true).commit()) {
            Log.w(TAG, "Could not persist the initial game-file access confirmation.");
            return false;
        }
        mInitialGameFilesConfirmed = true;
        cleanupUnusedStorageDirectory();
        return true;
    }

    private synchronized void cleanupUnusedStorageDirectory() {
        SharedPreferences preferences = getSharedPreferences(STORAGE_PREFS, MODE_PRIVATE);
        if (!mInitialGameFilesConfirmed || !mStorageModeSelected
                || !preferences.getBoolean(KEY_STORAGE_CLEANUP_PENDING, false)) {
            return;
        }
        // This is best-effort cleanup for new installs only. Never recurse or
        // delete files belonging to another installation.
        if (!mUseSharedStorage && !hasSharedStoragePermission()) {
            return;
        }
        try {
            File unused = mUseSharedStorage ? getExternalFilesDir(null)
                    : SHARStoragePaths.getSharedGameDirectory(this);
            if (unused == null) {
                return;
            }
            if (unused.exists()) {
                String[] children = unused.list();
                if (children == null) {
                    return;
                }
                if (children.length == 0) {
                    if (!unused.delete()) {
                        Log.w(TAG, "Could not remove unused empty storage directory: " + unused);
                        return;
                    }
                } else {
                    Log.i(TAG, "Preserving non-empty alternative storage directory: " + unused);
                }
            }
            if (!preferences.edit().remove(KEY_STORAGE_CLEANUP_PENDING).commit()) {
                Log.w(TAG, "Could not persist storage cleanup completion.");
            }
        } catch (SecurityException e) {
            Log.w(TAG, "Cannot access the alternative storage directory for cleanup.", e);
        }
    }

    @Override
    protected void resumeNativeThread() {
        if (mBrokenLibraries) {
            super.resumeNativeThread();
            return;
        }

        mStorageReady = prepareGameStorage();
        if (!mStorageReady) {
            super.pauseNativeThread();
            showStorageScreen();
            return;
        }

        if (mStorageScreen != null) {
            mLayout.removeView(mStorageScreen);
            mStorageScreen = null;
        }
        super.resumeNativeThread();
    }

    @Override
    protected void onResume() {
        mActivityResumed = true;
        super.onResume();
        // SDL normally resumes in onStart on API 24+. Settings or permission
        // windows can return through onResume without a new onStart.
        if (mHasMultiWindow && !isNativeStartupReady()) {
            resumeNativeThread();
        }
    }

    @Override
    protected void onPause() {
        mActivityResumed = false;
        super.onPause();
    }

    private File getActiveStorageDirectory() {
        if (mUseSharedStorage) {
            return SHARStoragePaths.getSharedGameDirectory(this);
        }
        File legacyRoot = getExternalFilesDir(null);
        return legacyRoot != null ? legacyRoot
                : new File("/storage/emulated/0/Android/data/com.c4rlox.simpsons/files");
    }

    private static boolean hasLegacyGameAssets(File root) {
        if (root == null || !root.isDirectory()) {
            return false;
        }
        return new File(root, "scripts.rcf").isFile()
                || new File(root, "soundfx.rcf").isFile()
                || new File(root, "art/frontend/scrooby/bootup.p3d").isFile();
    }

    private static boolean hasInitialGameAssets(File root) {
        return isNonEmptyFile(new File(root, "scripts.rcf"))
                && isNonEmptyFile(new File(root, "soundfx.rcf"))
                && isNonEmptyFile(new File(root, "art/frontend/scrooby/bootup.p3d"));
    }

    private static boolean isNonEmptyFile(File file) {
        return file.isFile() && file.length() > 0;
    }

    private boolean ensureStorageDirectory(File root) {
        if (!root.isDirectory() && !root.mkdirs() && !root.isDirectory()) {
            mStorageError = getString(R.string.storage_directory_error);
            return false;
        }
        return true;
    }

    private boolean hasSharedStoragePermission() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            return Environment.isExternalStorageManager();
        }
        return checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE)
                    == PackageManager.PERMISSION_GRANTED
                && checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE)
                    == PackageManager.PERMISSION_GRANTED;
    }

    private boolean prepareGameStorage() {
        try {
            return prepareGameStorageChecked();
        } catch (SecurityException e) {
            Log.e(TAG, "Storage access denied.", e);
            mStorageError = getString(R.string.storage_directory_error);
            return false;
        }
    }

    private boolean prepareGameStorageChecked() {
        mStorageError = getString(R.string.storage_directory_error);
        if (!selectStorageMode()) {
            return false;
        }
        if (mUseSharedStorage && !hasSharedStoragePermission()) {
            mStorageError = getString(R.string.storage_permission_message);
            return false;
        }

        // After the native loader confirms basic file access, skip this one-time
        // storage setup. Missing assets are always handled by the original engine.
        // Storage permission is still required for shared-folder access.
        if (mInitialGameFilesConfirmed || mStorageReady) {
            cleanupUnusedStorageDirectory();
            return true;
        }

        File root = getActiveStorageDirectory();
        try {
            if (!ensureStorageDirectory(root)) {
                return false;
            }

            // Test actual write access, not just File.canWrite().
            File probe = File.createTempFile(".shar-write-test-", ".tmp", root);
            if (!probe.delete()) {
                Log.w(TAG, "Could not remove write probe: " + probe);
            }
        } catch (IOException | SecurityException e) {
            Log.e(TAG, "Storage is unavailable: " + root, e);
            mStorageError = getString(R.string.storage_directory_error);
            return false;
        }

        Log.i(TAG, "Using " + (mUseSharedStorage ? "shared" : "legacy") + " storage: " + root);
        return true;
    }

    private void showStorageScreen() {
        boolean creatingScreen = mStorageScreen == null;
        if (creatingScreen) {
            mStorageScreen = new ScrollView(this);
            mStorageScreen.setBackgroundColor(Color.BLACK);

            LinearLayout content = new LinearLayout(this);
            content.setOrientation(LinearLayout.VERTICAL);
            content.setGravity(Gravity.CENTER_HORIZONTAL);
            int padding = Math.round(24 * getResources().getDisplayMetrics().density);
            content.setPadding(padding, padding, padding, padding);

            mStorageMessage = new TextView(this);
            mStorageMessage.setId(View.generateViewId());
            mStorageMessage.setTextColor(Color.WHITE);
            mStorageMessage.setTextSize(18);
            mStorageMessage.setTextIsSelectable(true);
            content.addView(mStorageMessage);

            mPermissionButton = new Button(this);
            mPermissionButton.setId(View.generateViewId());
            mPermissionButton.setText(R.string.storage_grant_permission);
            mPermissionButton.setOnClickListener(view -> requestSharedStoragePermission());
            content.addView(mPermissionButton);

            mExitButton = new Button(this);
            mExitButton.setId(View.generateViewId());
            mExitButton.setText(R.string.storage_exit);
            mExitButton.setOnClickListener(view -> finish());
            content.addView(mExitButton);

            // Keep native Android click handling for touch, Enter and D-pad center.
            for (Button button : new Button[] { mPermissionButton, mExitButton }) {
                button.setFocusable(true);
                button.setOnFocusChangeListener((view, focused) ->
                        view.setBackgroundTintList(ColorStateList.valueOf(
                                focused ? Color.rgb(255, 213, 79) : Color.LTGRAY)));
                button.setTextColor(Color.BLACK);
                button.setBackgroundTintList(ColorStateList.valueOf(Color.LTGRAY));
            }
            mPermissionButton.setNextFocusUpId(mStorageMessage.getId());
            mPermissionButton.setNextFocusDownId(mExitButton.getId());
            mExitButton.setNextFocusUpId(mPermissionButton.getId());

            mStorageScreen.addView(content);
            mLayout.addView(mStorageScreen, new ViewGroup.LayoutParams(
                    ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));
        }

        if (mAwaitingStorageChoice) {
            mStorageMessage.setText(getString(R.string.storage_choice_message,
                    SHARStoragePaths.getSharedGameDirectory(this).getAbsolutePath(),
                    getActiveStorageDirectory().getAbsolutePath(), mStorageError));
        } else {
            mStorageMessage.setText(getString(R.string.storage_setup_message,
                    getActiveStorageDirectory().getAbsolutePath(), mStorageError));
        }
        boolean showPermission = (mAwaitingStorageChoice || mUseSharedStorage)
                && !hasSharedStoragePermission();
        mPermissionButton.setVisibility(showPermission ? View.VISIBLE : View.GONE);
        mStorageMessage.setNextFocusDownId(showPermission
                ? mPermissionButton.getId() : mExitButton.getId());
        mExitButton.setNextFocusUpId(showPermission
                ? mPermissionButton.getId() : mStorageMessage.getId());
        if (creatingScreen || !mStorageScreen.hasFocus()) {
            (showPermission ? mPermissionButton : mExitButton).requestFocus();
        }
    }

    private void requestSharedStoragePermission() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            try {
                Intent intent = new Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION,
                        Uri.parse("package:" + getPackageName()));
                startActivityForResult(intent, REQUEST_STORAGE);
            } catch (ActivityNotFoundException | SecurityException e) {
                try {
                    startActivityForResult(new Intent(Settings.ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION),
                            REQUEST_STORAGE);
                } catch (ActivityNotFoundException | SecurityException unavailable) {
                    mStorageError = getString(R.string.storage_settings_error);
                    showStorageScreen();
                }
            }
        } else {
            requestPermissions(new String[] {
                    Manifest.permission.READ_EXTERNAL_STORAGE,
                    Manifest.permission.WRITE_EXTERNAL_STORAGE
            }, REQUEST_STORAGE);
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == REQUEST_STORAGE && mActivityResumed) {
            resumeNativeThread();
        }
    }

    // Returning from Settings triggers onResume -> resumeNativeThread, which checks
    // the permission again. Never trust the Settings activity's resultCode.
}
