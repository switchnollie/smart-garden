import React from "react";
import { IonApp } from "@ionic/react";
import { IonReactRouter } from "@ionic/react-router";
import { ThemeProvider } from "styled-components";
import { SWRConfig } from "swr";
import { TabsRoot } from "./pages";
import theme, { AppStyles } from "./theme";

function App() {
  return (
    <IonApp>
      <SWRConfig
        value={{
          refreshInterval: 30000,
          /* @ts-ignore */
          fetcher: (input: RequestInfo, init?: RequestInit | undefined) =>
            fetch(input, init).then((res) => res.json()),
        }}
      >
        <IonReactRouter>
          <AppStyles />
          <ThemeProvider theme={theme}>
            <TabsRoot />
          </ThemeProvider>
        </IonReactRouter>
      </SWRConfig>
    </IonApp>
  );
}

export default App;
