import React from "react";
import { Route, Redirect } from "react-router-dom";
import {
  IonPage,
  IonTabs,
  IonRouterOutlet,
  IonTabBar,
  IonTabButton,
  IonLabel,
} from "@ionic/react";
import { PlantsPage, InsightsPage, ProfilePage, PlantDetailsPage } from "./";
import Icon from "../components/Icon";

export default function TabsRoot() {
  return (
    <IonPage>
      <IonTabs>
        <IonRouterOutlet>
          <Route
            exact
            path="/app"
            render={() => <Redirect to="/app/plants" />}
          />
          <Route path="/app/:tab(plants)" exact component={PlantsPage} />
          <Route
            path="/app/:tab(plants)/:groupId"
            component={PlantDetailsPage}
          />
          <Route path="/app/:tab(insights)" exact component={InsightsPage} />
          <Route path="/app/:tab(profile)" exact component={ProfilePage} />
        </IonRouterOutlet>
        <IonTabBar slot="bottom" mode="ios">
          <IonTabButton tab="plants" href="/app/plants">
            <Icon height={20} icon="seed" />
            <IonLabel>Plants</IonLabel>
          </IonTabButton>
          <IonTabButton tab="insights" href="/app/insights">
            <Icon height={20} icon="insights" />
            <IonLabel>Insights</IonLabel>
          </IonTabButton>
          <IonTabButton tab="profile" href="/app/profile">
            <Icon height={20} icon="profile" />
            <IonLabel>Profile</IonLabel>
          </IonTabButton>
        </IonTabBar>
      </IonTabs>
    </IonPage>
  );
}
