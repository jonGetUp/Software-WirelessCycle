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

/**
 * For Internal Use only !
 * @author Patrice Rudaz (patrice.rudaz@hevs.ch)
 * @version 1.0 - 2017/06/15
 */
public interface DeviceInformationServiceListener {
    /** Callback of the Device Information Service */
    void onManufacturerNameRead(DeviceInformationService service, String manufacturerName);

    void onModelNumberRead(DeviceInformationService service, String modelNumber);

    void onSerialNumberRead(DeviceInformationService service, String serialNumber);

    void onHardwareRevisionRead(DeviceInformationService service, String hardwareRevision);

//    void onSoftwareRevisionRead(DeviceInformationService service, String softwareRevision);

    void onFirmwareRevisionRead(DeviceInformationService service, String firmwareRevision);
}
