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
package ch.hevs.heatflow.ble.services.hevs.mhfs;

import android.support.annotation.Nullable;

import java.nio.ByteBuffer;

/**
 * For Internal Use only !
 * @author Patrice Rudaz (patrice.rudaz@hevs.ch)
 * @version 1.0 - 2017/06/15
 */
public interface MhfsServiceListener {

    /**
     * Callback method to notify any listener about changes on ADS Configuration's characteristic.
     *
     * @param service       The current service (MHFS - Monitor HeatFlow Service)
     * @param index         The identifier of the ADS to work with
     * @param configuration The new configuration of the given ADS
     * @param error         An error if something went wrong, or {@code null} otherwise.
     */
    void onAdsConfigurationChanged(MhfsService service, Byte index, Short configuration, @Nullable Error error);

    /**
     * Callback method to notify any listener about changes on Command's characteristic.
     *
     * @param service       The current service (MHFS - Monitor HeatFlow Service)
     * @param command       The new command
     * @param error         An error if something went wrong, or {@code null} otherwise.
     */
    void onCommandChanged(MhfsService service, Byte command, @Nullable Error error);

    /**
     * Callback method to notify any listener about changes on ADS Values' characteristic.
     * @param service       The current service (MHFS - Monitor HeatFlow Service)
     * @param adsId         The identifier of the ADS to work with
     * @param mux           The MUX configuration of the selected ADS
     * @param pga           The PGA configuration of the selected ADS
     * @param isFloatValue  Indicates if the given value is a Float or a Integer value
     * @param value         The value
     * @param error         If an error occurs...
     */
    void onAdsValuesChanged(MhfsService service, byte adsId, byte mux, byte pga, boolean isFloatValue, Integer value, @Nullable Error error);

    /**
     * Callback method to notify any listener about the notification's state on the ADS Values char.
     * @param service       The current service (MHFS - Monitor HeatFlow Service)
     * @param enable        TO know if the notification is enable or not.
     */
    void onAdsValuesNotificationChanged(MhfsService service, boolean enable);
}
