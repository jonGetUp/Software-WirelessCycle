/* ************************************************************************************************
 * Copyright (c) 2017, Soprod SA, HES-SO Valais-Wallis, HEI, Infotronics
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
package ch.hevs.heatflow.ble.utils;

import ch.hevs.ble.lib.exceptions.BleScanException;
import ch.hevs.ble.lib.scanner.DiscoveredEntity;
import ch.hevs.ble.lib.scanner.Scanner;

public final class HfBleConstants {

    /**
     * Key used to pass the {@link DiscoveredEntity} descriptor as intent argument.
     * The descriptor of the discovered Soprod Watch is serialized and passed between activities.
     */
    public final static String HF_EXTRA_INTENT_KEY_DESC = "ch.hevs.heatflow.key_extra_desc";

    /**
     * Default Bluetooth connection timeout. If this timeout is reached, the callback
     * {@link ch.hevs.heatflow.ble.discovery.HfDiscoveryListener#onDiscoveryError(Scanner, BleScanException)}  is called.
     * <p>
     * The default timeout on Android is 30 seconds. Using the BLE library, the maximum timeout value
     * is fixed to 25 seconds.
     * When connecting the first time to a Soprod Watch, the connection time can be up to 5 seconds because the pairing
     * process can take time and it is based on some internal delays.
     */
    public final static int HF_BLE_CONN_TIMEOUT_MS = 25000;

    /**
     * Defines the timeout default value for the Widget function
     */
    public static final int HF_MHFS_RESPONSE_TIMEOUT_MS = 8000;
    /**
     * Defines the maximum time between two data packets
     */
    public static final int HF_MHFS_RECEIVE_TIMEOUT_MS = 3000;

}
