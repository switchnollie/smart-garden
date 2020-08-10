import React, { ReactNode } from "react";
import styled, { css } from "styled-components";
import { IonHeader } from "@ionic/react";

const Title = styled.h1`
  min-height: 3.125rem;
`;

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
      <Title>{children}</Title>
    </IonHeader>
  );
}
