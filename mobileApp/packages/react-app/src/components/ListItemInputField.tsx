import React, { useState, ChangeEvent } from "react";
import LabeledStat from "./LabeledStat";
import { ListItem } from "./List";
import { IonButtons, IonButton, IonIcon } from "@ionic/react";
import Icon from "./Icon";
import theme from "../theme";
import { checkmark } from "ionicons/icons";

interface ListItemInputFieldProps {
  value: string;
  onChange?: (e: ChangeEvent<HTMLInputElement>) => any;
  statColor: "primary" | "secondary";
  label: string;
  valueSuffix?: string;
}

export default function ListItemInputField({
  value,
  onChange,
  statColor,
  label,
  valueSuffix,
}: ListItemInputFieldProps) {
  const [editing, setEditing] = useState(false);
  return (
    <ListItem lines="none">
      <LabeledStat
        primary={statColor === "primary"}
        secondary={statColor === "secondary"}
        value={value}
        label={label}
        isTextInput={editing}
        onChange={onChange}
        valueSuffix={valueSuffix}
      />
      <IonButtons slot="end">
        <IonButton onClick={() => setEditing(!editing)}>
          {editing ? (
            <IonIcon color={theme.colors.font.fontPrimary} icon={checkmark} />
          ) : (
            <Icon color={theme.colors.font.fontPrimary} icon="edit" />
          )}
        </IonButton>
      </IonButtons>
    </ListItem>
  );
}
