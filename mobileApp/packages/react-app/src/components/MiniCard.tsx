import styled, { css } from "styled-components";

interface MiniCardProps {
  primary?: boolean;
  secondary?: boolean;
  disabled?: boolean;
  children: string;
  onClick?: (e: React.MouseEvent<HTMLDivElement, MouseEvent>) => void;
}

const MiniCard = styled.div<MiniCardProps>`
  ${({ theme, secondary, disabled }) => css`
    background: ${secondary
      ? theme.colors.gradients.blue
      : theme.colors.gradients.blueish};
    border-radius: 0.4375rem;
    opacity: ${disabled ? 0.5 : 1};
    color: ${theme.colors.font.fontPrimary};
    font-weight: bold;
    display: flex;
    justify-content: center;
    align-items: center;
    font-size: 1rem;
    min-height: 7.375rem;
    box-shadow: 0px 1px 4px 0px rgba(0, 0, 0, 0.08),
      0px 3px 16px 0px rgba(0, 0, 0, 0.06);
  `}
`;

export default MiniCard;
