package ch.hevs.heatflow.ble.error;

import java.util.Locale;

public final class MhfsCommandTimeoutError extends Error {
    private int mTimeout = -1;

    public MhfsCommandTimeoutError(int timeout) {
        super(String.format(Locale.getDefault(), "Did not get response in %d [ms].", timeout));
        mTimeout = timeout;
    }


    public int getTimeout() {
        return mTimeout;
    }
}
