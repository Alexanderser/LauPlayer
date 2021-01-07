package com.foxconn.lau.myapplication;

public class BsPatch {

    static {
        System.loadLibrary("native-lib");
    }

    public native void BsPatcher(String oldApk, String patch, String newApk);

}
