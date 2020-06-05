package ch.hevs.heatflow.ble.error;

public final class MhfsAdsValuesError extends Error {
    private String mReason = "Unknown reason";

    public MhfsAdsValuesError(String reason) {
        super("No valid ADS Values received. Reason: " + reason);
        mReason = reason;
    }

    public String getReason() { return mReason; }
}
