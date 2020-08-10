import React, { useEffect, useState, ReactElement } from "react";
import theme from "../theme";
import {
  IonContent,
  IonButton,
  IonButtons,
  IonGrid,
  IonRow,
  IonCol,
} from "@ionic/react";
import Header from "../components/Header";
import { RouteComponentProps } from "react-router-dom";
import { WateringGroupDetails } from "../types/WateringGroupDetails";
import List, { ListItem, ListHeader } from "../components/List";
import LabeledStat from "../components/LabeledStat";
import Icon from "../components/Icon";
import MiniCard from "../components/MiniCard";
import { Device } from "../types/Device";

interface PlantsPageProps extends RouteComponentProps<{ groupId: string }> {}

const mockData: WateringGroupDetails = {
  devices: [
    {
      logBuckets: ["5f2d6fb18fd7805f913cc40a"],
      _id: "5f2d2f46c254098c1222a484",
      displayName: "Moisture Sensor 1",
      ownedBy: "5f2d2b58d65dd0c3e0ac05e7",
      type: "moisture",
    },
    {
      logBuckets: ["5f2d728d1407c5e120a3980e"],
      _id: "5f2d2f515e9536fb08962ba5",
      displayName: "Pump 1",
      ownedBy: "5f2d2b58d65dd0c3e0ac05e7",
      type: "pump",
    },
    {
      logBuckets: ["5f2d72f56a21f09e868acf71"],
      _id: "5f2d30f7c22d4d3103a19b22",
      displayName: "Waterlevel Sensor 1",
      ownedBy: "5f2d2b58d65dd0c3e0ac05e7",
      type: "waterlevel",
    },
  ],
  _id: "5f2d2bfe7824f2b9fd33cb66",
  displayName: "Orchid",
  ownedBy: "5f2d2b58d65dd0c3e0ac05e7",
  lastPumped: "2020-08-08T15:29:06.494Z",
  moistureThreshold: 55,
};

export default function PlantDetailsPage({ match }: PlantsPageProps) {
  const [
    wateringGroup,
    setWateringGroup,
  ] = useState<WateringGroupDetails | null>(null);
  useEffect(() => {
    // Simulate network call
    setTimeout(() => {
      setWateringGroup(mockData);
    }, 300);
  }, [match.params.groupId]);

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
              <ListItem lines="none">
                <LabeledStat
                  primary
                  value={`${Math.round(
                    (wateringGroup.moistureThreshold / 1023) * 100
                  )}%`}
                  label="Watering Threshold"
                />
                <IonButtons slot="end">
                  <IonButton>
                    <Icon color={theme.colors.font.fontPrimary} icon="edit" />
                  </IonButton>
                </IonButtons>
              </ListItem>
              <ListItem lines="none">
                <LabeledStat
                  primary
                  value="once a week"
                  label="Pump at least"
                />
                <IonButtons slot="end">
                  <IonButton>
                    <Icon color={theme.colors.font.fontPrimary} icon="edit" />
                  </IonButton>
                </IonButtons>
              </ListItem>
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
