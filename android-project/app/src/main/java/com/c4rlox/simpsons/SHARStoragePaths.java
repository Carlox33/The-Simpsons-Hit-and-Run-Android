package com.c4rlox.simpsons;

import android.content.Context;

import java.io.File;

public final class SHARStoragePaths {
    private SHARStoragePaths() {
    }

    public static File getSharedGameDirectory(Context context) {
        File fallback = new File("/storage/emulated/0/c4rloxSimpsons");
        File root;
        try {
            // SDL_AndroidGetExternalStoragePath uses this same Android API.
            root = context.getExternalFilesDir(null);
        } catch (SecurityException e) {
            return fallback;
        }
        if (root == null || !root.isAbsolute()) {
            return fallback;
        }

        // Ascend files -> package -> data -> Android -> storage root.
        // Use parent components rather than passing ".." to filesystem APIs.
        String[] expected = { "files", null, "data", "Android" };
        for (String name : expected) {
            if (root == null || root.getName().isEmpty()
                    || ".".equals(root.getName()) || "..".equals(root.getName())
                    || (name != null && !name.equals(root.getName()))) {
                return fallback;
            }
            root = root.getParentFile();
        }
        if (root == null || root.getParentFile() == null) {
            return fallback;
        }
        return new File(root, "c4rloxSimpsons");
    }
}
