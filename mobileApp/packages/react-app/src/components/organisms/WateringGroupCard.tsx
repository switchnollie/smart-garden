import React from "react";
import Card, {
  CardHeader,
  CardSubtitle,
  CardTitle,
  CardContent,
} from "../Card";
import { IonGrid, IonCol, IonRow } from "@ionic/react";
import CountUpStat from "../CountUpStat";

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
              <CountUpStat value={32} primary label="Moisture" />
            </IonCol>
            <IonCol size="4">
              <CountUpStat value={5} secondary label="Water Level" />
            </IonCol>
          </IonRow>
        </IonGrid>
      </CardContent>
    </Card>
  );
}
