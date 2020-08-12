import React from "react";
import styled, { css } from "styled-components";
import LabeledStat from "./LabeledStat";
import { ListItem } from "./List";
import { IonSelect, IonSelectOption } from "@ionic/react";

interface ListItemSelectFieldProps {
  value: string;
  onChange?: (event: CustomEvent<any>) => void;
  statColor: "primary" | "secondary";
  label: string;
  valueSuffix?: string;
  onSubmit?: () => any;
}

const Select = styled(IonSelect)`
  ${({ theme }) => css`
    &::part(icon) {
      color: ${theme.colors.font.fontPrimary};
    }
    &::part(text) {
      display: none;
    }
  `}
`;

const selectOptions: { value: number; description: string }[] = [
  { value: 1000 * 60 * 60 * 24 * 7, description: "once a week" },
  { value: 1000 * 60 * 60 * 24, description: "daily" },
];

export default function ListItemSelectField({
  value,
  onChange,
  statColor,
  label,
  valueSuffix,
}: ListItemSelectFieldProps) {
  return (
    <ListItem lines="none">
      <LabeledStat
        primary={statColor === "primary"}
        secondary={statColor === "secondary"}
        value={
          selectOptions.find((o) => o.value.toString() === value)
            ?.description || ""
        }
        label={label}
        valueSuffix={valueSuffix}
      />
      <Select
        value={value}
        okText="Okay"
        cancelText="Dismiss"
        onIonChange={(e) => {
          onChange && onChange(e);
        }}
      >
        {selectOptions.map((option) => (
          <IonSelectOption
            key={option.description}
            value={option.value.toString()}
          >
            {option.description}
          </IonSelectOption>
        ))}
      </Select>
    </ListItem>
  );
}
