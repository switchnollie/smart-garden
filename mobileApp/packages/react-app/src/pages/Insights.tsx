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
          console.log(group.devices);
          const lastMoistureValues = group.devices
            .filter((device) => device.type === "moisture")
            .map((device) => device.lastValue);
          const hasMoistureValues =
            !!lastMoistureValues &&
            Array.isArray(lastMoistureValues) &&
            lastMoistureValues.length > 0;
          const avrgLastMoistureValue = hasMoistureValues
            ? lastMoistureValues.reduce((a, b) => a + b) /
              lastMoistureValues.length
            : 0;
          console.log(group.devices);
          const lastWaterLevelValue = group.devices
            .filter((device) => device.type === "waterlevel")
            .map((device) => device.lastValue);
          const hasWaterlevelValues =
            !!lastWaterLevelValue &&
            Array.isArray(lastWaterLevelValue) &&
            lastWaterLevelValue.length > 0;
          const avrgLastWaterLevelValue = hasWaterlevelValues
            ? lastWaterLevelValue.reduce((a, b) => a + b) /
              lastWaterLevelValue.length
            : 0;

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
