/**
 * Alipay.com Inc. Copyright (c) 2004-2019 All Rights Reserved.
 */
package com.alipay.kepler.fasterkv;

/**
 * @author taomeng
 * @version $Id: FasterKvObject.java, v 0.1 2019.01.21 20:56 taomeng Exp $
 */
public class FasterKvObject {

    protected final long nativeHandle_;

    protected FasterKvObject(final long nativeHandle) {
        this.nativeHandle_ = nativeHandle;
    }


}