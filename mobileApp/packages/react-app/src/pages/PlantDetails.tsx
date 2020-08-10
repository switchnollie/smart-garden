import React from "react";
import { IonContent } from "@ionic/react";
import Header from "../components/Header";
import Card from "../components/Card";
import { RouteComponentProps } from "react-router-dom";

interface PlantsPageProps extends RouteComponentProps<{ groupName: string }> {}

export default function PlantsPage({ match }: PlantsPageProps) {
  return (
    <>
      <Header>{match.params.groupName}</Header>
      <IonContent>
        <Card />
      </IonContent>
    </>
  );
}
