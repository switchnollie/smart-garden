import React, { ReactElement, useState } from "react";
import produce from "immer";
import { mutate } from "swr";
import { RouteComponentProps } from "react-router-dom";
import { IonContent, IonGrid, IonRow, IonCol } from "@ionic/react";
import Header from "../components/Header";
import List, { ListItem, ListHeader } from "../components/List";
import MiniCard from "../components/MiniCard";
import ListItemInputField from "../components/ListItemInputField";
import { Device } from "../types/Device";
import useWateringGroups from "../hooks/useWateringGroups";
import {
  getTenBitPercentage,
  getTenBitAnalog,
} from "../utils/relativeAnalogValues";
import { WateringGroup } from "../types/WateringGroup";

interface PlantsPageProps extends RouteComponentProps<{ groupId: string }> {}

export default function PlantDetailsPage({ match }: PlantsPageProps) {
  const { groups } = useWateringGroups();
  const [moistureThreshold, setMoistureThreshold] = useState<number | null>(
    null
  );

  let wateringGroup: WateringGroup | undefined;
  // Initialize wateringGroup and moistureThreshold
  if (groups) {
    wateringGroup = groups.find((group) => group._id === match.params.groupId);
    if (moistureThreshold === null && wateringGroup?.moistureThreshold) {
      setMoistureThreshold(
        getTenBitPercentage(wateringGroup.moistureThreshold)
      );
    }
  }

  const handleMoistureSubmit = async () => {
    if (groups && wateringGroup && moistureThreshold) {
      const uri = "/api/wateringgroup";
      const tenBitMoistureThreshold = getTenBitAnalog(moistureThreshold);
      const selectedGroupIndex = groups.findIndex(
        (group) => group._id === match.params.groupId
      );
      // Only update if changed
      if (tenBitMoistureThreshold !== wateringGroup.moistureThreshold) {
        const updatedData = produce(groups, (draft) => {
          draft[selectedGroupIndex].moistureThreshold = tenBitMoistureThreshold;
        });
        // Update local cache immediately
        mutate(uri, updatedData, false);
        // Send update API Request
        const res = await fetch(`${uri}/${wateringGroup._id}`, {
          method: "PUT",
          headers: {
            "Content-Type": "application/json",
          },
          body: JSON.stringify({ moistureThreshold: tenBitMoistureThreshold }),
        });
      }
    }
  };

  const groupDevices = (devices: Device[]) => {
    const columns = devices.map((device) => (
      <IonCol key={device._id} size="6">
        <MiniCard
          secondary={device.type === "moisture" || device.type === "waterlevel"}
        >
          {device.displayName}
        </MiniCard>
      </IonCol>
    ));

    const rows: Array<ReactElement[]> = [];
    columns.forEach((column, index) => {
      if (index % 2 === 0) {
        rows.push([column]);
      } else {
        rows[rows.length - 1].push(column);
      }
    });
    return rows.map((columns, index) => <IonRow key={index}>{columns}</IonRow>);
  };
  return (
    <>
      <Header backButton>
        {wateringGroup ? wateringGroup.displayName : ""}
      </Header>
      <IonContent>
        {wateringGroup && (
          <>
            <List>
              <ListHeader>
                <h3>Configuration</h3>
              </ListHeader>
              <ListItemInputField
                label="Watering Threshold"
                statColor="primary"
                value={moistureThreshold ? moistureThreshold.toString() : ""}
                valueSuffix="%"
                onChange={(e) =>
                  setMoistureThreshold(parseInt(e.target.value, 10))
                }
                onSubmit={handleMoistureSubmit}
              />
              <ListItemInputField
                label="Pump at least"
                statColor="primary"
                value="once a week"
              />
            </List>
            <List>
              <ListHeader>
                <h3>Devices</h3>
              </ListHeader>
              <ListItem lines="none">
                <IonGrid>{groupDevices(wateringGroup.devices)}</IonGrid>
              </ListItem>
            </List>
          </>
        )}
      </IonContent>
    </>
  );
}
