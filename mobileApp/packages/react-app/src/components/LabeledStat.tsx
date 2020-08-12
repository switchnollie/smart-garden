import React, { ChangeEvent, forwardRef } from "react";
import styled, { css } from "styled-components";
import { useCountUp } from "react-countup";
import AutosizeInput from "react-input-autosize";

interface LabeledStatProps {
  primary?: boolean;
  secondary?: boolean;
  value: number | string;
  label: string;
  isTextInput?: boolean;
  onChange?: (e: ChangeEvent<HTMLInputElement>) => any;
  valueSuffix?: string;
}

type LabeledStatHocProps = LabeledStatProps & { countUp?: boolean };

const getColor = ({
  primary,
  secondary,
}: Pick<LabeledStatProps, "primary" | "secondary">) => {
  if (primary) {
    return "var(--ion-color-primary)";
  } else if (secondary) {
    return "var(--ion-color-danger)";
  }
  return "var(--ion-text-color)";
};

const Container = styled.div<any>`
  ${({ primary, secondary, theme }) => css`
    input,
    h2,
    span.suffix {
      font-size: 2.25rem !important;
      font-weight: bold !important;
      color: ${getColor({ primary, secondary })};
      line-height: 1;
      margin-top: 2px;
      margin-bottom: 2px;
      background: none;
      border: none;
      outline: none;
      padding: 0;
    }
  `}
`;
const CountLabel = styled.h4`
  ${({ theme }) => css`
    font-size: 0.875rem !important;
    color: ${theme.colors.font.fontPrimary};
    font-weight: bold !important;
    margin-top: 2px;
    margin-bottom: 2px;
  `}
`;

const LabeledStat = forwardRef<HTMLInputElement, LabeledStatProps>(
  (
    { primary, secondary, value, label, isTextInput, onChange, valueSuffix },
    ref
  ) => (
    <Container primary={primary} secondary={secondary}>
      {isTextInput ? (
        <>
          <AutosizeInput ref={ref as any} value={value} onChange={onChange} />
          {valueSuffix && <span className="suffix">{valueSuffix}</span>}
        </>
      ) : (
        <h2>{value + (valueSuffix || "")}</h2>
      )}
      <CountLabel>{label}</CountLabel>
    </Container>
  )
);

function CountUpStat({ value, valueSuffix, ...props }: LabeledStatProps) {
  const { countUp: counterValue } = useCountUp({
    start: 0,
    duration: 0.7,
    decimals: 0,
    delay: 0.5,
    suffix: valueSuffix,
    end: value as number,
  });
  return <LabeledStat {...props} value={counterValue} />;
}

export default ({ countUp, value, ...props }: LabeledStatHocProps) =>
  countUp && !isNaN(value as any) ? (
    <CountUpStat value={value} {...props} />
  ) : (
    <LabeledStat value={value} {...props} />
  );
