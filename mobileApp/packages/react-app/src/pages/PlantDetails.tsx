import React, { useEffect, useState } from "react";
import { IonContent } from "@ionic/react";
import Header from "../components/Header";
import { RouteComponentProps } from "react-router-dom";
import { WateringGroupDetails } from "../types/WateringGroupDetails";

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
  return (
    <>
      <Header>{wateringGroup ? wateringGroup.displayName : ""}</Header>
      <IonContent>Details here</IonContent>
    </>
  );
}
