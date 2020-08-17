import React from "react";
import { IonReactRouter } from "@ionic/react-router";
import { ThemeProvider } from "styled-components";
import { SWRConfig } from "swr";
import { TabsRoot } from "./pages";
import theme, { AppStyles } from "./theme";
import ProtectedRoute from "./auth/ProtectedRoute";
import { useSessionContext } from "./contexts/SessionContext";
import { Switch, Route, Redirect } from "react-router-dom";
import Login from "./pages/Login";

function App() {
  const [sessionContext] = useSessionContext();
  return (
    <SWRConfig
      value={{
        refreshInterval: 30000,
        /* @ts-ignore */
        fetcher: (input: RequestInfo, init?: RequestInit | undefined) =>
          fetch(input, {
            headers: {
              Authorization: sessionContext.jwt || "",
              ...init?.headers,
            },
            ...init,
          }).then((res) => res.json()),
      }}
    >
      <IonReactRouter>
        <AppStyles />
        <ThemeProvider theme={theme}>
          <Switch>
            <Route exact path="/" render={() => <Redirect to="/app" />} />
            <ProtectedRoute path="/app" redirectPath="/login">
              <TabsRoot />
            </ProtectedRoute>
            <Route path="/login" component={Login} />
          </Switch>
        </ThemeProvider>
      </IonReactRouter>
    </SWRConfig>
  );
}

export default App;
