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
package ch.hevs.heatflow.scanner;

import android.content.res.Resources;
import android.support.annotation.NonNull;
import android.support.v7.widget.CardView;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

import ch.hevs.ble.lib.scanner.DiscoveredEntity;
import ch.hevs.ble.lib.scanner.OnEntityClickListener;

import ch.hevs.heatflow.R;
import ch.hevs.heatflow.ble.core.BleDeviceDescriptor;

/**
 * Created by rut on 19/10/17.
 */

public class EntityAdapter extends RecyclerView.Adapter<EntityAdapter.EntityViewHolder> implements AdapterView.OnItemClickListener {

    private List<BleDeviceDescriptor> mEntities = new ArrayList<>();
    private OnEntityClickListener mListener;

    /**
     * Create a custom adapter to display {@link DiscoveredEntity} ina list.
     *
     * @param listener the listener used to be notified when the user select an entity
     */
    public EntityAdapter(@NonNull OnEntityClickListener listener) {
        setHasStableIds(true);
        mListener = listener;
    }

    /**
     * Display a new discovered entity in the list.
     *
     * @param entity the entity to add and display
     */
    public void addEntity(BleDeviceDescriptor entity) {
        if (entity == null)
            return;

        // Update the information available int the advertising frame if the device has been already discovered
        if (mEntities.contains(entity)) {
            // Update the advertising information only
            final BleDeviceDescriptor desc = mEntities.get(mEntities.indexOf(entity));
            desc.setAdvertisementInfo(entity.rssi(), entity.scanRecord());
            desc.setAdvertisingMode(entity.getAdvertisingMode());
        }
        // Add a new discovered device
        else {
            mEntities.add(entity);
        }

        notifyDataSetChanged();
    }

    public void removeEntity(BleDeviceDescriptor entity) {
        if (entity == null)
            return;

        // Remove the watch if it is inside the list
        mEntities.remove(entity);

        notifyDataSetChanged();
    }

    /**
     * Remove all existing entities currently displayed.
     */
    public void clear() {
        mEntities.clear();
        notifyDataSetChanged();
    }

    @Override
    public int getItemCount() {
        return mEntities.size();
    }

    @Override
    public long getItemId(int position) {
        return mEntities.get(position).hashCode();
    }

    @Override
    public EntityViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View v = LayoutInflater.from(parent.getContext()).inflate(R.layout.device_row_item, parent, false);
        return new EntityViewHolder(v, this);
    }

    @Override
    public void onBindViewHolder(EntityViewHolder holder, int position) {
        final BleDeviceDescriptor desc = mEntities.get(position);

        // Check if the entity has a name or not
        String entityName = desc.name();
        if (entityName == null || entityName.length() == 0)
            entityName = Resources.getSystem().getString(android.R.string.unknownName);

        // Display the entity information in the row list
        holder.deviceName.setText(entityName);
        holder.deviceInfo.setText(String.format(Locale.getDefault(), "%s (%s)", desc.address(), desc.getAdvertisingMode().name()));
        holder.deviceRSSI.setText(String.format(Locale.US, "%d %s", desc.rssi(), "dBm"));
    }

    @Override
    public void onItemClick(AdapterView<?> adapterView, View view, int i, long l) {
        // An entity has been clicked. Return the selected entity.
        if (i < 0)
            return; // Ignore if NO_POSITION is returned

        mListener.onEntitySelected(mEntities.get(i));
    }

    static class EntityViewHolder extends RecyclerView.ViewHolder implements View.OnClickListener {
        private CardView mCard;
        private AdapterView.OnItemClickListener mListener;
        TextView deviceName, deviceInfo, deviceRSSI;

        EntityViewHolder(View v, AdapterView.OnItemClickListener listener) {
            super(v);
            mCard = (CardView) itemView.findViewById(R.id.device_row_card);
            deviceName = (TextView) itemView.findViewById(R.id.device_name);
            deviceInfo = (TextView) itemView.findViewById(R.id.device_address);
            deviceRSSI = (TextView) itemView.findViewById(R.id.device_rssi);

            mListener = listener;
            mCard.setOnClickListener(this);
        }

        @Override
        public void onClick(View view) {
            // Handle row click. Only use the position.
            mListener.onItemClick(null, null, getAdapterPosition(), 0); // NO_POSITION can be returned
        }
    }

}
