/*
 * *************************************************************************************************
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
 * *************************************************************************************************
 */
package ch.hevs.heatflow.ble.services.hevs;

import java.util.UUID;

/**
 * @author Patrice Rudaz (patrice.rudaz@hevs.ch, patrice.rudaz@soprod.ch)
 * @version 1.0 - 2017/06/15
 */
public abstract class HevsUuid {

    /**
     * UUID base number for Soprod
     *
     * This UUID is build on the company name "Soprod", its location (Sion) and the company ID
     * assigned by the Bluetooth SIG (0x0512).
     *
     * 0x64 0x6F 0x72 0x70 0x6F 0x53 0xFF 0xFF 0x6E 0x6F 0x69 0x53 0x00 0x00 0x00 0x00
     *  d    o    r    p    o    S      ...      n    o    i    S           ...

     */
    private final static String HEVS_PATTERN = "0000*-CAFE-025A-1700-556F41535653";

    protected static UUID fromString(final String id) {
        return UUID.fromString(HEVS_PATTERN.replace("*", id));
    }
}
