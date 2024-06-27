#include "AuxFunctions.h"
#include <cmath>

vector<vector<string>> AuxFunctions::maxWaterPerCity;

AuxFunctions::AuxFunctions() = default;

void AuxFunctions::testAndVisit(std::queue<Vertex*> &q, Edge* e, Vertex* w, double residual) {
    if (!w->isVisited() && residual > 0) {
        w->setVisited(true);
        w->setPath(e);
        q.push(w);
    }
}

bool AuxFunctions::findAugmentingPaths(Vertex* s, Vertex* t) {
    for(Vertex* v : csvInfo::pipesGraph.getVertexSet()) {
        v->setVisited(false);
    }
    s->setVisited(true);
    std::queue<Vertex*> q;
    q.push(s);
    while(!q.empty() && !t->isVisited()) {
        auto v = q.front();
        q.pop();
        for(Edge* e: v->getAdj()) {
            testAndVisit(q, e, e->getDest(), e->getWeight() - e->getFlow());
        }
        for(Edge* e: v->getIncoming()) {
            testAndVisit(q, e, e->getOrig(), e->getFlow());
        }
    }
    return t->isVisited();
}

double AuxFunctions::findMinResidualAlongPath(Vertex* s, Vertex* t) {
    double f = INF;
    for (Vertex* v = t; v != s;) {
        Edge* e = v->getPath();
        if (e->getDest() == v) {
            f = std::min(f, e->getWeight() - e->getFlow());
            v = e->getOrig();
        }
        else {
            f = std::min(f, e->getFlow());
            v = e->getDest();
        }
    }
    return f;
}

void AuxFunctions::augmentFlowAlongPath(Vertex* s, Vertex* t) {
    double f = findMinResidualAlongPath(s, t);
    for (Vertex* v = t; v != s;) {
        Edge* e = v->getPath();
        double flow = e->getFlow();
        if (e->getDest() == v) {
            e->setFlow(std::min(flow + f, e->getWeight()));
            v = e->getOrig();
        }
        else {
            e->setFlow(std::max(flow - f, 0.0));
            v = e->getDest();
        }
    }
}

void AuxFunctions::edmondsKarp(const string& source, const string& target) {
    Vertex* s = csvInfo::pipesGraph.findVertex(source);
    Vertex* t = csvInfo::pipesGraph.findVertex(target);
    while (findAugmentingPaths(s, t)) {
        augmentFlowAlongPath(s, t);
    }
}

void AuxFunctions::MaxWaterCity() {
    vector<string>  aux;
    for (Vertex* v : csvInfo::pipesGraph.getVertexSet()) {
        for (Edge* e : v->getAdj()) {
            e->setFlow(0);
        }
    }

    AuxFunctions::edmondsKarp("super_source", "super_sink");

    for (auto & idx : csvInfo::citiesVector) {
        Vertex* s = csvInfo::pipesGraph.findVertex(idx.getCode());
        double flow = s->getAdj()[0]->getFlow();
        aux.clear();
        aux.push_back(idx.getCity());
        aux.push_back(idx.getCode());
        aux.push_back(std::to_string(static_cast<long long>(std::round(flow))));
        maxWaterPerCity.push_back(aux);
    }
}

void AuxFunctions::MaxFlow(bool csv) {
    maxWaterPerCity.clear();

    // add super sink
    csvInfo::pipesGraph.addVertex("super_sink", -1, -1);
    for (Vertex* v : csvInfo::pipesGraph.getVertexSet()) {
        if (v->getType() == 0) {
            csvInfo::pipesGraph.addEdge(v->getInfo(), "super_sink", csvInfo::citiesVector[v->getPos()].getDemand() * 1.0);
        }
    }

    // add super source
    csvInfo::pipesGraph.addVertex("super_source", -1, -1);
    for (Vertex* v : csvInfo::pipesGraph.getVertexSet()) {
        if (v->getType() == 1) {
            csvInfo::pipesGraph.addEdge("super_source", v->getInfo(), csvInfo::reservoirsVector[v->getPos()].getMaxDelivery() * 1.0);
        }
    }

    AuxFunctions::MaxWaterCity();
    if (csv) csvInfo::writeToMaxWaterPerCity(maxWaterPerCity);

    csvInfo::pipesGraph.removeVertex("super_sink");
    csvInfo::pipesGraph.removeVertex("super_source");
}

void AuxFunctions::simulateReservoirRemoval(const std::string& reservoirCode) {
    Vertex* reservoirVertex = csvInfo::pipesGraph.findVertex(reservoirCode);

    for (Edge* edge : reservoirVertex->getAdj()) {
        edge->setWeight(0);
    }

    MaxFlow(false);

    for (Edge* edge : reservoirVertex->getAdj()) {
        edge->setWeight(edge->getCapacity());
    }
}

void AuxFunctions::simulatePumpingStationRemoval(const string& code){
    Vertex* v = csvInfo::pipesGraph.findVertex(code);
    for(auto e : v->getAdj()){
            e->setWeight(0);
    }

    MaxFlow(false);

    for(auto e : v->getAdj()){
          e->setWeight(e->getCapacity());
    }
}

void AuxFunctions::simulatePipelineFailure(Edge* e){
    e->setWeight(0);

    MaxFlow(false);

    e->setWeight(e->getCapacity());
}

void AuxFunctions:: testAndVisit_ReservoirRemovalPart(std::queue<Vertex*> &q, Edge* e, Vertex* w, double flow) {
    if (!w->isVisited() && flow > 0) {
        w->setVisited(true);
        w->setPath(e);
        q.push(w);
    }
}

bool AuxFunctions::findAugmentingPaths_ReservoirRemovalPart(Vertex* s, Vertex* t, bool residual) {
    for(Vertex* v : csvInfo::pipesGraph.getVertexSet()) {
        v->setVisited(false);
    }
    if (residual) {
        if ((s->getIncoming()[0]->getWeight() - s->getIncoming()[0]->getFlow() == 0) ||
            (t->getAdj()[0]->getWeight() - t->getAdj()[0]->getFlow() == 0)) {
            return false;
        }
    }
    else {
        if ((s->getIncoming()[0]->getFlow() == 0) || (t->getAdj()[0]->getFlow() == 0)) {
            return false;
        }
    }
    s->setVisited(true);
    std::queue<Vertex*> q;
    q.push(s);
    while(!q.empty() && !t->isVisited()) {
        auto v = q.front();
        q.pop();
        for(Edge* e: v->getAdj()) {
            if (residual) testAndVisit(q, e, e->getDest(), e->getWeight() - e->getFlow());
            else testAndVisit_ReservoirRemovalPart(q, e, e->getDest(), e->getFlow());
        }
    }
    return t->isVisited();
}

double AuxFunctions::findMinflow(Vertex *r, Vertex *c) {
    Vertex* current = c;
    double m = INF;
    while (current != r) {
        Edge* e = current->getPath();
        if (e->getFlow() < m) m = e->getFlow();
        current = e->getOrig();
    }
    m = min(r->getIncoming()[0]->getFlow(), m);
    m = min(c->getAdj()[0]->getFlow(), m);
    return m;
}

void AuxFunctions::removeFlow(Vertex* r, Vertex* sink) {
    Vertex* current = sink;
    double f = findMinflow(r, sink);
    while (current != r) {
        Edge* e = current->getPath();
        e->setFlow(max(e->getFlow() - f, 0.0));
        current = e->getOrig();
    }
    r->getIncoming()[0]->setFlow(r->getIncoming()[0]->getFlow() - f);
    sink->getAdj()[0]->setFlow(sink->getAdj()[0]->getFlow() - f);
}

void AuxFunctions::simulateReservoirRemovalPart(const string& code) {
    maxWaterPerCity.clear();

    // add super sink
    csvInfo::pipesGraph.addVertex("super_sink", -1, -1);
    for (Vertex* v : csvInfo::pipesGraph.getVertexSet()) {
        if (v->getType() == 0) {
            csvInfo::pipesGraph.addEdge(v->getInfo(), "super_sink", csvInfo::citiesVector[v->getPos()].getDemand() * 1.0);
        }
    }

    // add super source
    csvInfo::pipesGraph.addVertex("super_source", -1, -1);
    for (Vertex* v : csvInfo::pipesGraph.getVertexSet()) {
        if (v->getType() == 1) {
            csvInfo::pipesGraph.addEdge("super_source", v->getInfo(), csvInfo::reservoirsVector[v->getPos()].getMaxDelivery() * 1.0);
        }
    }
    Vertex* super_source = csvInfo::pipesGraph.findVertex("super_source");
    Vertex* super_sink = csvInfo::pipesGraph.findVertex("super_sink");

    AuxFunctions::MaxWaterCity();

    set<string> affected;
    Vertex* r = csvInfo::pipesGraph.findVertex(code);
    for (City c : csvInfo::citiesVector) {
        Vertex* sink = csvInfo::pipesGraph.findVertex(c.getCode());
        while (findAugmentingPaths_ReservoirRemovalPart(r, sink, false)) {
            affected.insert(c.getCode());
            removeFlow(r, sink);
        }
    }

    for (Reservoir rev : csvInfo::reservoirsVector) {
        Vertex* reservoir = csvInfo::pipesGraph.findVertex(rev.getCode());
        if (rev.getCode() != code) {
            for (const string& c : affected) {
                Vertex* city = csvInfo::pipesGraph.findVertex(c);
                while (findAugmentingPaths_ReservoirRemovalPart(reservoir, city, true)) {
                    reservoir->setPath(reservoir->getIncoming()[0]);
                    super_sink->setPath(city->getAdj()[0]);
                    augmentFlowAlongPath(super_source, super_sink);
                }
            }
        }
    }

    // restore graph's weights
    for (Vertex* v : csvInfo::pipesGraph.getVertexSet()) {
        for (Edge *e: v->getAdj()) {
            e->setWeight(e->getCapacity());
        }
    }

    maxWaterPerCity.clear();
    vector<string> aux;
    for (auto & idx : csvInfo::citiesVector) {
        Vertex* s = csvInfo::pipesGraph.findVertex(idx.getCode());
        double flow = 0;
        for (Edge* e : s->getIncoming()) flow += e->getFlow();
        aux.clear();
        aux.push_back(idx.getCity());
        aux.push_back(idx.getCode());
        aux.push_back(std::to_string(static_cast<long long>(std::round(flow))));
        maxWaterPerCity.push_back(aux);
    }

    csvInfo::pipesGraph.removeVertex("super_sink");
    csvInfo::pipesGraph.removeVertex("super_source");
}



vector<double> AuxFunctions::compute_metrics() {
    double num = 0.0;
    double sum = 0.0;
    double max = 0.0;
    double aux;
    double maxFlow = 0.0;

    for (Vertex* v : csvInfo::pipesGraph.getVertexSet()) {
        if (v->getType() == 0) {
            for (Edge* e : v->getIncoming()) {
                maxFlow += e->getFlow();
            }
        }
    }


    for (Vertex* v : csvInfo::pipesGraph.getVertexSet()) {
        for (Edge* e : v->getAdj()) {
            if (e->getFlow() == 0 & e->getReverse() != nullptr) continue;
            aux = e->getWeight() - e->getFlow();
            sum += aux;
            num++;
            if (aux > max) max = aux;
        }
    }

    double avg = sum * 1.0 / num;
    double var_diff = 0;
    for (Vertex* v : csvInfo::pipesGraph.getVertexSet()) {
        for (Edge* e : v->getAdj()) {
            if (e->getFlow() == 0 & e->getReverse() != nullptr) continue;
            var_diff += pow(e->getWeight() - e->getFlow() - avg, 2);
        }
    }
    var_diff /= num;

    vector<double> metrics;
    metrics = {maxFlow, avg, var_diff, max};
    return metrics;
}

void AuxFunctions::print_metrics(vector<double> i, vector<double> f) {
    cout << "          initial >> final" << endl;
    cout << "max flow: " << i[0] << " >> " << f[0] << endl;
    cout << "average:  " << i[1] << " >> " << f[1] << endl;
    cout << "variance: " << i[2] << " >> " << f[2] << endl;
    cout << "max diff: " << i[3] << " >> " << f[3] << endl;
}

bool AuxFunctions::findAugmentingPaths_balance(Vertex* s, Vertex* t, double delta) {
    for(Vertex* v : csvInfo::pipesGraph.getVertexSet()) {
        v->setVisited(false);
    }
    s->setVisited(true);
    std::queue<Vertex*> q;
    q.push(s);
    while(!q.empty() && !t->isVisited()) {
        auto v = q.front();
        q.pop();
        for(Edge* e: v->getAdj()) {
            if (e->getWeight() - e->getFlow() >= delta) {
                testAndVisit(q, e, e->getDest(), e->getWeight() - e->getFlow());
            }
        }
        for (Edge* e: v->getIncoming()) {
            if (e->getWeight() - e->getFlow() >= delta) {
                testAndVisit(q, e, e->getOrig(), e->getFlow());
            }
        }
    }
    return t->isVisited();
}

void AuxFunctions::augmentFlowAlongPath_CS(Vertex* s, Vertex* t) {
    double f = findMinResidualAlongPath(s, t);
    for (Vertex* v = t; v != s;) {
        Edge* e = v->getPath();
        double flow = e->getFlow();
        if (e->getDest() == v) {
            e->setFlow(std::min(flow + f, e->getWeight()));
            v = e->getOrig();
        }
        else {
            e->setFlow(std::max(flow - f, 0.0));
            v = e->getDest();
        }
        for (Edge* edge : e->getDest()->getAdj()) {
            if (edge->getDest() == e->getOrig()) {
                edge->setWeight(0);
            }
        }
    }
}

void AuxFunctions::balanceNetwork() {
    maxWaterPerCity.clear();

    // add super sink
    csvInfo::pipesGraph.addVertex("super_sink", -1, -1);
    for (Vertex* v : csvInfo::pipesGraph.getVertexSet()) {
        if (v->getType() == 0) {
            csvInfo::pipesGraph.addEdge(v->getInfo(), "super_sink", csvInfo::citiesVector[v->getPos()].getDemand() * 1.0);
        }
    }
    Vertex* super_sink = csvInfo::pipesGraph.findVertex("super_sink");

    // add super source
    csvInfo::pipesGraph.addVertex("super_source", -1, -1);
    for (Vertex* v : csvInfo::pipesGraph.getVertexSet()) {
        if (v->getType() == 1) {
            csvInfo::pipesGraph.addEdge("super_source", v->getInfo(), csvInfo::reservoirsVector[v->getPos()].getMaxDelivery() * 1.0);
        }
    }
    Vertex* super_source = csvInfo::pipesGraph.findVertex("super_source");

    for (Vertex* v : csvInfo::pipesGraph.getVertexSet()) {
        for (Edge* e : v->getAdj()) {
            e->setFlow(0);
        }
    }

    double maxW = AuxFunctions::maxWeight();

    double delta = AuxFunctions::compute_delta(maxW);

    while (delta >= 1) {
        while (findAugmentingPaths_balance(super_source, super_sink, delta))
            augmentFlowAlongPath_CS(super_source, super_sink);
        delta /= 2;
    }

    for (Vertex* v : csvInfo::pipesGraph.getVertexSet()) {
        for (Edge *edge: v->getAdj()) {
            edge->setWeight(edge->getCapacity());
        }
    }

    // remove super sink
    csvInfo::pipesGraph.removeVertex("super_sink");
    // remove super source
    csvInfo::pipesGraph.removeVertex("super_source");

}

double AuxFunctions::compute_delta(double maxW) {
    double delta = 0;
    for (int i = 0; pow(2, i) < maxW; i++) {
        delta = pow(2, i);
    }
    return delta;
}

double AuxFunctions::maxWeight() {
    double m = 0.0;
    for (Vertex* v : csvInfo::pipesGraph.getVertexSet()) {
        for (Edge* e : v->getAdj()) {
            m = max(m, e->getWeight());
        }
    }
    return m;
}
