import React, { ReactNode } from "react";
import { IonHeader } from "@ionic/react";

interface HeaderProps {
  backwardsLink?: string;
  children: string;
  actionButton?: ReactNode;
}

export default function Header({
  backwardsLink,
  actionButton,
  children,
}: HeaderProps) {
  return (
    <IonHeader mode="ios">
      <h1>{children}</h1>
    </IonHeader>
  );
}
