import React from "react";
import { IonContent } from "@ionic/react";
import Header from "../components/Header";
import WateringGroupCard from "../components/WateringGroupCard";

export default function InsightsPage() {
  return (
    <>
      <Header>Insights</Header>
      <IonContent>
        <WateringGroupCard name="Orchid" type="Watering Group" />
      </IonContent>
    </>
  );
}
