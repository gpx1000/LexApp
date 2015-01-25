package com.elasmotech.givingthroughglass.lexapp;

import android.test.InstrumentationTestCase;

/**
 * <a href="http://d.android.com/tools/testing/testing_android.html">Testing Fundamentals</a>
 */
public class ApplicationTest extends InstrumentationTestCase {
    public void test() throws Exception {
        final int expected = 5;
        final int reality = 5;
        assertEquals(expected, reality);
    }
}