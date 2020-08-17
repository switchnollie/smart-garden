import React from "react";
import { Redirect, Route, RouteProps } from "react-router";
import { useSessionContext } from "../contexts/SessionContext";

export interface ProtectedRouteProps extends RouteProps {
  redirectPath: string;
}

export default function ProtectedRoute({
  redirectPath,
  children,
  ...props
}: ProtectedRouteProps) {
  const [sessionContext] = useSessionContext();
  const isAuthenticated = !!sessionContext.isAuthenticated;
  return (
    <Route
      {...props}
      render={({ location }) =>
        isAuthenticated ? (
          children
        ) : (
          <Redirect
            to={{
              pathname: redirectPath,
              state: { from: location },
            }}
          />
        )
      }
    />
  );
}
