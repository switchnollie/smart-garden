import React from "react";
import moment from "moment";
import Card, { CardHeader, CardSubtitle, CardTitle, CardContent } from "./Card";
import { IonGrid, IonCol, IonRow } from "@ionic/react";
import LabeledStat from "./LabeledStat";
import { getTenBitPercentage } from "../utils/relativeAnalogValues";

interface WateringGroupCardProps {
  name: string;
  type: string;
  moisture: number; // 10 Bit, not a percentage value
  waterLevel: number; // 10 Bit, not a percentage value
  lastPumped: string; // Serialized ISO Date
}

export default function WateringGroupCard({
  name,
  type,
  moisture,
  waterLevel,
  lastPumped,
}: WateringGroupCardProps) {
  const relativeWaterLevel = getTenBitPercentage(waterLevel);
  const relativeMoisture = getTenBitPercentage(moisture);
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
                value={relativeMoisture}
                primary={relativeMoisture >= 10}
                secondary={relativeMoisture < 10}
                valueSuffix="%"
                label="Moisture"
              />
            </IonCol>
            <IonCol size="4">
              <LabeledStat
                countUp
                value={relativeWaterLevel}
                secondary={relativeWaterLevel < 10}
                primary={relativeWaterLevel >= 10}
                label="Water Level"
                valueSuffix="%"
              />
            </IonCol>
          </IonRow>
          <IonRow>
            <IonCol size="12">
              <LabeledStat
                value={moment(lastPumped).fromNow()}
                label="Last Pumped"
              />
            </IonCol>
          </IonRow>
        </IonGrid>
      </CardContent>
    </Card>
  );
}
