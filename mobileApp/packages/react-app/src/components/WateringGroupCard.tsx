import React from "react";
import Card, { CardHeader, CardSubtitle, CardTitle, CardContent } from "./Card";
import { IonGrid, IonCol, IonRow } from "@ionic/react";
import LabeledStat from "./LabeledStat";

interface WateringGroupCardProps {
  name: string;
  type: string;
}

export default function WateringGroupCard({
  name,
  type,
}: WateringGroupCardProps) {
  return (
    <Card>
      <CardHeader>
        <CardSubtitle>{type}</CardSubtitle>
        <CardTitle>{name}</CardTitle>
      </CardHeader>

      <CardContent>
        <IonGrid>
          <IonRow>
            <IonCol size="4">
              <LabeledStat
                countUp
                value={32}
                primary
                valueSuffix="%"
                label="Moisture"
              />
            </IonCol>
            <IonCol size="4">
              <LabeledStat
                countUp
                value={5}
                secondary
                label="Water Level"
                valueSuffix="%"
              />
            </IonCol>
          </IonRow>
          <IonRow>
            <IonCol size="12">
              <LabeledStat value="12 hours ago" label="Last Pumped" />
            </IonCol>
          </IonRow>
        </IonGrid>
      </CardContent>
    </Card>
  );
}
