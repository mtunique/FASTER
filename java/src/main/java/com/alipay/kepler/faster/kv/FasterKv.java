/**
 * Alipay.com Inc. Copyright (c) 2004-2019 All Rights Reserved.
 */
package com.alipay.kepler.faster.kv;

import java.util.concurrent.atomic.AtomicReference;

/**
 * @author taomeng
 * @version $Id: FasterKv.java, v 0.1 2019年01月21日 14:33 taomeng Exp $
 */
public class FasterKv {

    private enum LibraryState {
        NOT_LOADED,
        LOADING,
        LOADED
    }

    private static AtomicReference<LibraryState> libraryLoaded = new AtomicReference<>(LibraryState.NOT_LOADED);

    static {
        FasterKv.loadLibrary();
    }

    /**
     * Loads the necessary library files. Calling this method twice will have no effect. By default the method extracts the shared library
     * for loading at java.io.tmpdir, however, you can override this temporary location by setting the environment variable
     * FasterKv_SHAREDLIB_DIR.
     */
    public static void loadLibrary() {
        if (libraryLoaded.get() == LibraryState.LOADED) {
            return;
        }

        if (libraryLoaded.compareAndSet(LibraryState.NOT_LOADED, LibraryState.LOADING)) {
            System.loadLibrary("libfasterjni.a");

            libraryLoaded.set(LibraryState.LOADED);
            return;
        }

        while (libraryLoaded.get() == LibraryState.LOADING) {
            try {
                Thread.sleep(10);
            } catch (final InterruptedException e) {
                //ignore
            }
        }
    }

    public native static long open(final long tableSize, final long logSize, final String path, final double logMutableFraction);

    public static void main(String[] args) {
        open(1000, 1000, "test", 0.9);
    }

}