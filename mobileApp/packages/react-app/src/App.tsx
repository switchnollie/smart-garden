import React from "react";
import { IonApp } from "@ionic/react";
import { IonReactRouter } from "@ionic/react-router";
import { ThemeProvider } from "styled-components";
import { TabsRoot } from "./pages";
import theme, { AppStyles } from "./theme";

function App() {
  return (
    <IonApp>
      <IonReactRouter>
        <AppStyles />
        <ThemeProvider theme={theme}>
          <TabsRoot />
        </ThemeProvider>
      </IonReactRouter>
    </IonApp>
  );
}

export default App;
