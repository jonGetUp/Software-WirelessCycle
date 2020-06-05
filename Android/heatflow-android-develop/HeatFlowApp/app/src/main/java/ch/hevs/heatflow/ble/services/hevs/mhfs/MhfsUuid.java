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

import java.util.UUID;
import ch.hevs.heatflow.ble.services.hevs.HevsUuid;


/**
 * For Internal Use only !
 * @author Patrice Rudaz (patrice.rudaz@hevs.ch)
 * @version 1.0 - 2018/05/29
 */
public final class MhfsUuid extends HevsUuid {

    /**
     * Soprod SWS private BLE service (Soprod Widget Service)
     */
    public final static UUID SERVICE_MHFS = fromString("1011");

    /**
     * MHFS ADS values characteristic
     */
    public final static UUID CHAR_MHFS_ADS_VAL = fromString("1031");

    /**
     * MHFS ADS Configuration
     */
    public final static UUID CHAR_ADS_CONFIGURATION = fromString("1041");

    /**
     * MHFS HeatFlow command
     */
    public final static UUID CHAR_MHFS_CMD = fromString("1051");
}
