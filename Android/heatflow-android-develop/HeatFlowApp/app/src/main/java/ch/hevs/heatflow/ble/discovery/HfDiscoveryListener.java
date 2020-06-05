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


import android.support.annotation.NonNull;

import ch.hevs.ble.lib.exceptions.BleScanException;
import ch.hevs.ble.lib.scanner.Scanner;
import ch.hevs.heatflow.ble.core.BleDeviceDescriptor;

/**
 * Scanner listener interface used to indicate when a BLE device has been found by the scanner.
 * <p>
 * This interface is also used to report the status or an error of the scanner.
 *
 * @author Patrice Rudaz (patrice.rudaz@hevs.ch)
 * @version 1.0 - 2019/02/25
 */
public interface HfDiscoveryListener {

    /**
     * An entity with a specific manufacturer data has been discovered/found.
     * <p>
     * Note that this method can be called more than once for the very same entity.
     *
     * @param scanner    the scanner that actually discovered the entity
     * @param descriptor the descriptor of the discovered BLE device.
     */
    void onSpecificDeviceFound(@NonNull Scanner scanner, @NonNull BleDeviceDescriptor descriptor);

    /**
     * An entity has been discovered/found.
     * <p>
     * Note that this method can be called more than once for the very same entity.
     *
     * @param scanner    the scanner that actually discovered the entity
     * @param descriptor the descriptor of the discovered BLE device.
     */
    void onDeviceDiscovered(@NonNull Scanner scanner, @NonNull BleDeviceDescriptor descriptor);

    /**
     * Indicate when the scanning is stopped.
     * <p>
     * When called, the scanning has been stopped. This can be a manual stop or a scanning timeout.
     *
     * @param scanner the stopped scanner
     */
    void onDiscoveryStopped(@NonNull Scanner scanner);

    /**
     * An error occurred when starting the scanner.
     *
     * @param scanner   the error scanner
     * @param throwable the scanning error (with and error code)
     */
    void onDiscoveryError(@NonNull Scanner scanner, @NonNull BleScanException throwable);
}
