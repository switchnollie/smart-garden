import React, { useState, ChangeEvent, useRef } from "react";
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
  onSubmit?: () => any;
}

export default function ListItemInputField({
  value,
  onChange,
  statColor,
  label,
  valueSuffix,
  onSubmit,
}: ListItemInputFieldProps) {
  const inputRef = useRef<HTMLInputElement>();
  const [editing, setEditing] = useState(false);

  const changeEditingMode = () => {
    const newIsEditing = !editing;
    setEditing(newIsEditing);
    if (newIsEditing) {
      setTimeout(() => {
        inputRef?.current?.focus();
      }, 5);
    } else {
      onSubmit && onSubmit();
    }
  };
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
        ref={inputRef as any}
      />
      <IonButtons slot="end">
        <IonButton onClick={changeEditingMode}>
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
