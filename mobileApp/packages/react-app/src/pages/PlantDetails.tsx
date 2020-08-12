import React, { ReactElement, useState } from "react";
import { IonContent, IonGrid, IonRow, IonCol } from "@ionic/react";
import Header from "../components/Header";
import { RouteComponentProps } from "react-router-dom";
import List, { ListItem, ListHeader } from "../components/List";
import MiniCard from "../components/MiniCard";
import { Device } from "../types/Device";
import useWateringGroups from "../hooks/useWateringGroups";
import ListItemInputField from "../components/ListItemInputField";

interface PlantsPageProps extends RouteComponentProps<{ groupId: string }> {}

export default function PlantDetailsPage({ match }: PlantsPageProps) {
  const { groups } = useWateringGroups();
  const [moistureThreshold, setMoistureThreshold] = useState<number | null>(
    null
  );

  let wateringGroup;
  // Initialize wateringGroup and moistureThreshold
  if (groups) {
    wateringGroup = groups.find((group) => group._id === match.params.groupId);
    if (moistureThreshold === null && wateringGroup?.moistureThreshold) {
      setMoistureThreshold(
        Math.round((wateringGroup.moistureThreshold / 1023) * 100)
      );
    }
  }

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
