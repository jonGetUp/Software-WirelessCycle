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
package ch.hevs.heatflow.ble.services.adopted;

import java.util.UUID;

/**
 * For Internal Use only !
 * Assigned Numbers for the BLE adopted service. This is defined by tthe Bluetooth SIG
 *
 * @author Patrice Rudaz (patrice.rudaz@hevs.ch)
 * @version 1.0 - 2017/06/15
 */
public abstract class BtSigUuid {

    /**
     * UUID base number of the Bluetooth SIG
     */
    private final static String BLUETOOTH_SIG_PATTERN = "0000*-0000-1000-8000-00805f9b34fb";

    /**
     * Battery service
     */
    public final static UUID SERVICE_BATTERY = fromString("180F");
    public final static UUID CHAR_BATTERY_LEVEL = fromString("2A19");


    /**
     * Device Information service
     */
    public final static UUID SERVICE_DEVICE_INFORMATION = fromString("180A");
    public final static UUID CHAR_MANUFACTURER_NAME = fromString("2A29");
    public final static UUID CHAR_MODEL_NUMBER = fromString("2A24");
    public final static UUID CHAR_SERIAL_NUMBER = fromString("2A25");
    public final static UUID CHAR_HARDWARE_REVISION = fromString("2A27");
    public final static UUID CHAR_FIRMWARE_REVISION = fromString("2A26");
    public final static UUID CHAR_SOFTWARE_REVISION = fromString("2A28");

    protected static UUID fromString(final String id) {
        return UUID.fromString(BLUETOOTH_SIG_PATTERN.replace("*", id));
    }
}
