import React, { ReactNode } from "react";
import theme from "../theme";
import styled from "styled-components";
import { IonHeader, IonBackButton, IonButtons, IonToolbar } from "@ionic/react";
import { chevronBack } from "ionicons/icons";

const Title = styled.h1`
  min-height: 3.125rem;
`;

const Buttons = styled(IonButtons)`
  margin-right: 1rem;
`;

const Toolbar = styled(IonToolbar)`
  --background: none;
  --border-style: none;
`;

interface HeaderProps {
  backButton?: boolean;
  children: string;
  actionButton?: ReactNode;
}

export default function Header({
  backButton,
  actionButton,
  children,
}: HeaderProps) {
  return (
    <IonHeader mode="ios">
      <Toolbar>
        {backButton && (
          <Buttons slot="start">
            <IonBackButton
              mode="ios"
              text=""
              icon={chevronBack}
              defaultHref="/"
              color={theme.colors.font.fontPrimary}
            />
          </Buttons>
        )}
        <Title>{children}</Title>
      </Toolbar>
    </IonHeader>
  );
}
