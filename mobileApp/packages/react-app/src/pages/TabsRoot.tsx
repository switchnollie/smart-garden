import React from "react";
import { PlantsPage, InsightsPage, ProfilePage, PlantDetailsPage } from "./";
import {
  IonPage,
  IonTabs,
  IonRouterOutlet,
  IonTabBar,
  IonTabButton,
  IonIcon,
  IonLabel,
} from "@ionic/react";
import { Route, Redirect } from "react-router-dom";

export default function TabsRoot() {
  return (
    <IonPage>
      <IonTabs>
        <IonRouterOutlet>
          <Route exact path="/" render={() => <Redirect to="/plants" />} />
          <Route path="/:tab(plants)" exact component={PlantsPage} />
          <Route path="/:tab(plants)/:groupName" component={PlantDetailsPage} />
          <Route path="/:tab(insights)" exact component={InsightsPage} />
          <Route path="/:tab(profile)" exact component={ProfilePage} />
        </IonRouterOutlet>
        <IonTabBar slot="bottom">
          <IonTabButton tab="plants" href="/plants">
            <IonIcon name="flash" />
            <IonLabel>Plants</IonLabel>
          </IonTabButton>
          <IonTabButton tab="insights" href="/insights">
            <IonIcon name="apps" />
            <IonLabel>Insights</IonLabel>
          </IonTabButton>
          <IonTabButton tab="profile" href="/profile">
            <IonIcon name="apps" />
            <IonLabel>Profile</IonLabel>
          </IonTabButton>
        </IonTabBar>
      </IonTabs>
    </IonPage>
  );
}
