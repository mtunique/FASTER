/**
 * Alipay.com Inc. Copyright (c) 2004-2019 All Rights Reserved.
 */
package com.alipay.kepler.fasterkv;

import java.security.SecureRandom;
import java.util.concurrent.atomic.AtomicReference;

/**
 * @author taomeng
 * @version $Id: FasterKv.java, v 0.1 2019.01.21 14:33 taomeng Exp $
 */
public class FasterKv extends FasterKvObject {

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
            System.loadLibrary("fasterkvjni");

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

    public native static long get(final long handler, byte[] key, final int len);

    public native static long put(final long handler, byte[] key, final int len, final int v);


    public FasterKv(long nativeHandle) {
        super(nativeHandle);
    }

    public void put(int k, int v) {
//        put(this.nativeHandle_, k, v);
    }

    public void put(byte[] k, int v) {
        put(this.nativeHandle_, k, k.length, v);
    }

    public void put(byte[] k, int len, int v) {
        put(this.nativeHandle_, k, len, v);
    }

    public long get(byte[] k) {
        return get(nativeHandle_, k, k.length);
    }

    public long get(byte[] k, int len) {
        return get(nativeHandle_, k, len);
    }

    private static final String ALPHABET = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_";
    private static final SecureRandom RANDOM = new SecureRandom();

    public static String generate(int count) {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < count; ++i) {
            sb.append(ALPHABET.charAt(RANDOM.nextInt(ALPHABET.length())));
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        FasterKv fasterKv = new FasterKv(open(128 * 128 * 16, 1073741824, "test", 0.9));
        System.out.println(fasterKv.nativeHandle_);

        long start = System.currentTimeMillis();
        for (int size = 1; size < 128; size++) {
            System.out.println(size);
            for (int i = 0; i < 10000; i++) {
                String rds = generate(size);

                for (int j = 0; j < size / 2; j++) {
                    fasterKv.put(rds.getBytes(), rds.getBytes().length - j, rds.getBytes()[j]);
                }

                for (int j = 0; j < size / 2; j++) {
                    if (fasterKv.get(rds.getBytes(), rds.getBytes().length - j) != rds.getBytes()[j]) {
                        System.out.println("error");
                    }

                }
            }
        }

        System.out.println(System.currentTimeMillis() - start);

//        fasterKv = new FasterKv(open(128, 1073741824, "test2", 0.9));
//        System.out.println(fasterKv.nativeHandle_);

//        for (int i = 0; i < 10; i++) {
//            fasterKv.put(Character.MAX_VALUE - i, Character.MAX_VALUE - i - 2);
//        }
//
//        for (int i = 0; i < 10; i++) {
//            if (fasterKv.get(Character.MAX_VALUE - i) != Character.MAX_VALUE - i - 2) {
//                System.out.println("error");
//            }
//        }
    }

}