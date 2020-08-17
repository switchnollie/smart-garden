import React from "react";
import { IonContent } from "@ionic/react";
import Header from "../components/Header";
import WateringGroupCard from "../components/WateringGroupCard";
import useWateringGroups from "../hooks/useWateringGroups";

export default function InsightsPage() {
  const { groups } = useWateringGroups();
  return (
    <>
      <Header>Insights</Header>
      <IonContent>
        {groups?.map((group) => {
          const lastMoistureValues = group.devices
            .filter((device) => device.type === "moisture")
            .map((device) => device.lastValue);
          const avrgLastMoistureValue =
            lastMoistureValues.reduce((a, b) => a + b) /
            lastMoistureValues.length;

          const lastWaterLevelValue = group.devices
            .filter((device) => device.type === "waterlevel")
            .map((device) => device.lastValue);
          const avrgLastWaterLevelValue =
            lastWaterLevelValue.reduce((a, b) => a + b) /
            lastWaterLevelValue.length;

          return (
            <WateringGroupCard
              key={group._id}
              moisture={avrgLastMoistureValue}
              waterLevel={avrgLastWaterLevelValue}
              lastPumped={group.lastPumped}
              name={group.displayName}
              type="Watering Group"
            />
          );
        })}
      </IonContent>
    </>
  );
}
