/* ************************************************************************************************
 * Copyright (c) 2019, HES-SO Valais-Wallis, HEI, Infotronics
 * All rights reserved.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
 * WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ************************************************************************************************** */
package ch.hevs.heatflow.ble.discovery;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 * @author Patrice Rudaz (patrice.rudaz@hevs.ch)
 * @version 1.0 - 2017/06/15
 */
public class HfManufacturerData {
    /**
     * This is the registered company ID for HE given by the Bluetooth SIG
     */
    public final static int BLE_HEVS_COMPANY_IDENTIFIER = 0x025A;


    private short mModelIdentifier  = 0x0000;
    private short mSoftwareRevision = 0x0000;
    private short mHardwareRevision = 0x0000;


    private HfManufacturerData(short modelId, short value1, short value2) {
        mModelIdentifier    = modelId;
        mSoftwareRevision   = value1;
        mHardwareRevision   = value2;
    }

    /**
     * Parse manufacturer data bytes to {@link ScanRecord#getManufacturerSpecificData(int)}.
     * <p>
     * The format is defined in Bluetooth 4.1 specification, Volume 3, Part C, Section 11 and 18.
     * <p>
     * All numerical multi-byte entities and values shall use little-endian <strong>byte</strong>
     * order.
     *
     * @param manufacturerData An array of bytes extracted from a scan record of Bluetooth LE advertisement.
     */
    public static HfManufacturerData parseFromBytes(byte[] manufacturerData) {
        short modelId   = 0;
        short reserved1 = 0;
        short reserved2 = 0;
        ByteBuffer bytes = ByteBuffer.wrap(manufacturerData);
        bytes.order(ByteOrder.LITTLE_ENDIAN);

        if (bytes.remaining() >= 2) {
            modelId = bytes.getShort();
        }

        if (bytes.remaining() >= 2) {
            reserved1 = bytes.getShort();
        }

        if (bytes.remaining() >= 2) {
            reserved2 = bytes.getShort();
        }

        return new HfManufacturerData(modelId, reserved1, reserved2);
    }

    /**
     * Returns the model identifier
     */
    public short getModelIdentifier() { return mModelIdentifier; }

    /**
     * Returns the Software revision
     */
    public short getSoftwareRevision() {
        return mSoftwareRevision;
    }
    /**
     * Returns the Software revision
     */
    public short getHardwareRevision() {
        return mHardwareRevision;
    }
}
