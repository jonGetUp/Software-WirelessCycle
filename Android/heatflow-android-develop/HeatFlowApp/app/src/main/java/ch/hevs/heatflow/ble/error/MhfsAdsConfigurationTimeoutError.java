package ch.hevs.heatflow.ble.error;

import java.util.Locale;

public final class MhfsAdsConfigurationTimeoutError extends Error {
    private int mTimeout = -1;

    public MhfsAdsConfigurationTimeoutError(int timeout) {
        super(String.format(Locale.getDefault(), "Did not get response in %d [ms].", timeout));
        mTimeout = timeout;
    }


    public int getTimeout() {
        return mTimeout;
    }
}
