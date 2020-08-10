import React from "react";
import styled, { css } from "styled-components";
import Icon from "./Icon";

interface IconButtonProps {
  icon: string;
  onClick?: (e: React.MouseEvent<HTMLButtonElement, MouseEvent>) => void;
}

const StyledButton = styled.button`
  ${({ theme }) => css`
    background-color: ${theme.colors.accentBlue};
    border-radius: 50%;
    color: ${theme.colors.font.fontPrimary};
    width: 3.625rem;
    height: 3.625rem;
    display: flex;
    justify-content: center;
    align-items: center;
    font-size: 1.1875rem;
    box-shadow: 0px 1px 4px 0px rgba(0, 0, 0, 0.08),
      0px 3px 16px 0px rgba(0, 0, 0, 0.06);
  `}
`;

export default function IconButton({ icon, onClick }: IconButtonProps) {
  return (
    <StyledButton onClick={onClick}>
      <Icon icon={icon} color="#fff" />
    </StyledButton>
  );
}
