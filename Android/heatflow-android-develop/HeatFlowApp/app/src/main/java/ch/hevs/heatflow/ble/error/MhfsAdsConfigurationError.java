package ch.hevs.heatflow.ble.error;

public final class MhfsAdsConfigurationError extends Error {
    private String mReason = "Unknown reason";

    public MhfsAdsConfigurationError(String reason) {
        super("No valid configuration received. Reason: " + reason);
        mReason = reason;
    }

    public String getReason() { return mReason; }
}
