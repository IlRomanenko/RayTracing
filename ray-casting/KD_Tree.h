//
// Created by ilya on 06.03.17.
//
#pragma once
#include <climits>

#include "../base_headers.h"
#include "../geometry/IGeometryObject.h"
#include "../geometry/BoundingBox.h"

#include "../utilities/ThreadPool.h"


class KD_Tree {

public:

    struct KD_tree_leaf {
        vector<IGeometryObject *> objects;
    };

    struct KD_tree_node {
        KD_tree_node *left, *right;
        BoundingBox boundingBox;
        KD_tree_leaf *isLeaf;
        pair<ldb, int> splitPlane;
        size_t nodeSize;

        KD_tree_node() {
            left = right = nullptr;
            isLeaf = nullptr;
            splitPlane = make_pair(0, 0);
            nodeSize = 0;
        }

        ~KD_tree_node() {
            if (left != nullptr) {
                delete left;
            }
            if (right != nullptr) {
                delete right;
            }
            if (isLeaf != nullptr) {
                delete isLeaf;
            }
        }
    };

    typedef KD_tree_node *pnode;

    struct BoundingBoxSplit {
        enum SplitType {
            LEFT, RIGHT
        };

        ldb value;
        size_t axis;
        SplitType type;
        ldb cost;

        BoundingBoxSplit() {
            value = 0;
            axis = 0;
            type = SplitType::LEFT;
            cost = INFINITY;
        }


    };

    const BoundingBox getBoundingBox(const vector<IGeometryObject *> &objects) const {
        BoundingBox result = objects[0]->getBoundingBox();
        for (const auto &obj : objects) {
            result.expand(obj->getBoundingBox());
        }
        return result;
    }


    ldb costFunction(ldb leftProbability, size_t leftSize, ldb rightProbability, size_t rightSize) const {
        return traversalCoef + intersectionCoef * (leftProbability * leftSize + rightProbability * rightSize);
    }


    pair<ldb, BoundingBoxSplit::SplitType> surfaceAreaHeuristic(ldb planeCoord, size_t axisNumber,
                                                                const BoundingBox &voxel,
                                                                size_t leftSize, size_t middleSize,
                                                                size_t rightSize) const {
        array<BoundingBox, 2> boxes = voxel.split(axisNumber, planeCoord);

        ldb leftProb = boxes[0].calculateSurfaceArea(), rightProb = boxes[1].calculateSurfaceArea(), fullProb =
                leftProb + rightProb;

        leftProb /= voxel.calculateSurfaceArea();
        rightProb /= voxel.calculateSurfaceArea();

        ldb leftPartCost = costFunction(leftProb, leftSize + middleSize, rightProb, rightSize);
        ldb rightPartCost = costFunction(leftProb, leftSize, rightProb, middleSize + rightSize);

        pair<ldb, BoundingBoxSplit::SplitType> result = make_pair(leftPartCost,
                                                                  BoundingBoxSplit::SplitType::LEFT);
        if (Double::greater(leftPartCost, rightPartCost)) {
            result = make_pair(rightPartCost, BoundingBoxSplit::SplitType::RIGHT);
        }

        //TODO maybe it isn't needed

        if (Double::equal(leftProb, 0) || Double::equal(rightProb, 0)) {
            result.first = INFINITY;
        }
        return result;
    }


    array<vector<IGeometryObject *>, 2> classify(const vector<IGeometryObject *> &objects,
                                                 const BoundingBox &left,
                                                 const BoundingBox &right,
                                                 BoundingBoxSplit bboxSplit) {
        //TODO change classify strategy!!!

        vector<IGeometryObject *> leftPart, rightPart;

        for (const auto &obj : objects) {
            const auto &bbox = obj->getBoundingBox();
            if (Double::equal(bbox.getMin(bboxSplit.axis), bbox.getMax(bboxSplit.axis)) &&
                Double::equal(bbox.getMin(bboxSplit.axis), bboxSplit.value)) {

                if (bboxSplit.type == BoundingBoxSplit::SplitType::LEFT) {
                    leftPart.push_back(obj);
                } else {
                    rightPart.push_back(obj);
                }

            } else {
                if (Double::less(bbox.getMin(bboxSplit.axis), bboxSplit.value)) {
                    leftPart.push_back(obj);
                }
                if (Double::less(bboxSplit.value, bbox.getMax(bboxSplit.axis))) {
                    rightPart.push_back(obj);
                }
            }
        }

        return {leftPart, rightPart};
    }

    struct IntersectionEvent {
        enum ObjectLocationType {
            BEGIN = 2, BELONGS = 1, END = 0
        };

        ObjectLocationType type;
        ldb value;

        IntersectionEvent(ldb value, ObjectLocationType type) {
            this->value = value;
            this->type = type;
        }

        bool operator<(const IntersectionEvent &other) const {
            return Double::less(value, other.value) || (Double::equal(value, other.value) && type < other.type);
        }
    };

    BoundingBoxSplit findPlane(const vector<IGeometryObject *> &objects, const BoundingBox &boundingBox) {
        BoundingBoxSplit boundingBoxPlaneSplit;
        ldb minCost = INFINITY;

        vector<IntersectionEvent> events;
        for (size_t dim = 0; dim < 3; dim++) {

            events.clear();
            for (const auto &obj : objects) {
                const auto &bbox = obj->getBoundingBox();
                // TODO
                if (Double::equal(bbox.getMin(dim), bbox.getMax(dim))) {
                    events.push_back(
                            IntersectionEvent(bbox.getMin(dim), IntersectionEvent::ObjectLocationType::BELONGS));
                } else {
                    events.push_back(
                            IntersectionEvent(bbox.getMin(dim), IntersectionEvent::ObjectLocationType::BEGIN));
                    events.push_back(
                            IntersectionEvent(bbox.getMax(dim), IntersectionEvent::ObjectLocationType::END));
                }
            }

            sort(events.begin(), events.end());

            size_t leftSize = 0, middleSize = 0, rightSize = objects.size();

            for (size_t i = 0; i < events.size(); i++) {
                size_t p_begin = 0, p_belong = 0, p_end = 0;

                ldb currentPlaneValue = events[i].value;

                while (i < events.size() && Double::equal(currentPlaneValue, events[i].value)) {
                    switch (events[i].type) {
                        case IntersectionEvent::BEGIN:
                            p_begin++;
                            break;
                        case IntersectionEvent::BELONGS:
                            p_belong++;
                            break;
                        case IntersectionEvent::END:
                            p_end++;
                            break;
                    }
                    i++;
                }
                i--;

                middleSize = p_belong;
                rightSize -= p_belong + p_end;

                const auto &currentResult = surfaceAreaHeuristic(currentPlaneValue, dim,
                                                                 boundingBox,
                                                                 leftSize, middleSize, rightSize);

                if (Double::less(currentResult.first, minCost)) {
                    minCost = currentResult.first;
                    boundingBoxPlaneSplit.value = currentPlaneValue;
                    boundingBoxPlaneSplit.axis = dim;
                    boundingBoxPlaneSplit.type = currentResult.second;
                    boundingBoxPlaneSplit.cost = minCost;
                }
                leftSize += p_belong + p_begin;

            }
        }
        return boundingBoxPlaneSplit;
    }

    pnode recBuild(const vector<IGeometryObject *> &objects, const BoundingBox &boundingBox) {
        pnode node = new KD_tree_node();
        node->boundingBox = boundingBox;
        node->nodeSize = objects.size();

        const auto &split = findPlane(objects, boundingBox);

        if (Double::greaterEqual(split.cost, objects.size() * intersectionCoef)) {
            node->isLeaf = new KD_tree_leaf();
            node->isLeaf->objects = objects;
            return node;
        }

        const auto &boxes = boundingBox.split(split.axis, split.value);
        const auto &parts = classify(objects, boxes[0], boxes[1], split);

        node->splitPlane = make_pair(split.value, split.axis);

        auto leftPart = bind(buildLambda, parts[0], boxes[0]);
        auto rightPart = bind(buildLambda, parts[1], boxes[1]);

        auto futureLeft = pool->submit(move(leftPart));

        node->right = rightPart();
        pool->wait(move(futureLeft));
        node->left = futureLeft.get();
        return node;
    }

    pair<bool, ldb> getPlaneSplitCoef(pnode node, const Ray &ray) {
        Vector normal;
        if (node->splitPlane.second == 0) {
            normal = Vector(1, 0, 0);
        } else if (node->splitPlane.second == 1) {
            normal = Vector(0, 1, 0);
        } else if (node->splitPlane.second == 2) {
            normal = Vector(0, 0, 1);
        }
        if (Double::equal(ray.direction * normal, 0)) {
            return make_pair(false, -1);
        }
        ldb coef = (node->splitPlane.first - ray.begin * normal) / (ray.direction * normal);
        return make_pair(true, coef);
    }

    Intersection findIntersection(pnode node, const Ray &ray, const array<ldb, 2> coefs) {

        if (node->nodeSize == 0 || !node->boundingBox.intersect(ray).first) {
            return Intersection();
        }

        if (node->isLeaf != nullptr) {
            Intersection intersection;
            ldb intersectionCoef = INFINITY;

            for (const auto &obj : node->isLeaf->objects) {
                auto coef_inters = obj->intersect(ray);
                if (coef_inters && Double::less(coef_inters.getIntersectionPointCoef(), intersectionCoef) &&
                    Double::greater(coef_inters.getIntersectionPointCoef(), 0)) {

                    intersectionCoef = coef_inters.getIntersectionPointCoef();

                    intersection.has_intersection = true;
                    intersection.rayIntersectionCoef = intersectionCoef;
                    intersection.object = obj;
                    intersection.intersectionPoint = ray.begin + ray.direction * intersectionCoef;
                }
            }
            return intersection;
        }

        Intersection left = findIntersection(node->left, ray, coefs);
        Intersection right = findIntersection(node->right, ray, coefs);
        if (left) {
            if (right && Double::greater(left.rayIntersectionCoef, right.rayIntersectionCoef) &&
                Double::greater(right.rayIntersectionCoef, 0)) {
                return right;
            }
            return left;
        }
        return right;
    }

public:

    KD_Tree() {
        root = nullptr;
        pool = nullptr;
    }

    KD_Tree(const vector<IGeometryObject *> &objects, size_t numberOfThreads = 8) {
        root = nullptr;
        pool = nullptr;
        buildTree(objects, numberOfThreads);
    }

    void buildTree(const vector<IGeometryObject *> &objects, size_t numberOfThreads = 8) {
        if (root != nullptr) {
            delete root;
            delete pool;
        }
        pool = new ThreadPool<pnode>(numberOfThreads - 1);
        auto result = pool->submit(bind(buildLambda, objects, getBoundingBox(objects)));
        pool->wait(move(result));
        root = result.get();
        pool->shutdown();
        delete pool;
    }

    Intersection castRay(const Ray &ray) {
        const auto &coefs = root->boundingBox.intersect(ray);
        if (!coefs.first) {
            return Intersection();
        }
        return findIntersection(root, ray,
                                {coefs.second[0].getIntersectionPointCoef(),
                                 coefs.second[1].getIntersectionPointCoef()});
    }

    ~KD_Tree() {
        delete root;
    }


public:

    const ldb traversalCoef = 2, intersectionCoef = 0.5;

    static const ldb INFINITY;

    pnode root;

    ThreadPool<pnode> *pool;

    const function<pnode(const vector<IGeometryObject*> &, const BoundingBox&)> buildLambda =
            [&](const vector<IGeometryObject *> &cur_objects, const BoundingBox &boundingBox) {
        return recBuild(cur_objects, boundingBox);
    };
};

const ldb KD_Tree::__builtin_inff() {
    return std::numeric_limits<double>::max();
}
