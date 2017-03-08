//
// Created by ilya on 06.03.17.
//
#pragma once

#include "../base_headers.h"
#include "../geometry/IGeometryObject.h"
#include "../geometry/BoundingBox.h"

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
            cost = 1e30;
        }


    };

    const ldb traversalCoef = 1, intersectionCoef = 10;


    const BoundingBox getBoundingBox(const vector<IGeometryObject *> &objects) const {
        BoundingBox result = objects[0]->getBoundingBox();
        for (const auto &obj : objects) {
            result.expand(obj->getBoundingBox());
        }
        return result;
    }


    ldb costFunction(ldb leftProbability, size_t leftSize, ldb rightProbability, size_t rightSize) const {
        ldb coef = 1;
        if (leftSize == 0 || rightSize == 0) {
            coef = 0.8;
        }
        return coef * (traversalCoef + intersectionCoef * (leftProbability * leftSize + rightProbability * rightSize));
    }


    pair<ldb, BoundingBoxSplit::SplitType> surfaceAreaHeuristic(ldb planeCoord, size_t axisNumber,
                                                                const BoundingBox &voxel,
                                                                size_t leftSize, size_t middleSize,
                                                                size_t rightSize) const {
        array<BoundingBox, 2> boxes = voxel.split(axisNumber, planeCoord);

        ldb voxelSA = voxel.calculateSurfaceArea();
        ldb leftProb = boxes[0].calculateSurfaceArea() / voxelSA, rightProb =
                boxes[1].calculateSurfaceArea() / voxelSA;

        ldb leftPartCost = costFunction(leftProb, leftSize + middleSize, rightProb, rightSize);
        ldb rightPartCost = costFunction(leftProb, leftSize, rightProb, middleSize + rightSize);

        pair<ldb, BoundingBoxSplit::SplitType> result = make_pair(leftPartCost,
                                                                  BoundingBoxSplit::SplitType::LEFT);
        if (Double::greater(leftPartCost, rightPartCost)) {
            result = make_pair(rightPartCost, BoundingBoxSplit::SplitType::RIGHT);
        }
/*
        if (leftSize + rightSize == 0) {
            result.first = 1e30;
        }*/
        return result;
    }


    array<vector<IGeometryObject *>, 2> classify(const vector<IGeometryObject *> &objects,
                                                 const BoundingBox &left,
                                                 const BoundingBox &right,
                                                 BoundingBoxSplit bboxSplit) {

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
                if (Double::lessEqual(bbox.getMin(bboxSplit.axis), bboxSplit.value)) {
                    leftPart.push_back(obj);
                }
                if (Double::lessEqual(bboxSplit.value, bbox.getMax(bboxSplit.axis))) {
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
        ldb minCost = 1e30;

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

                const auto &currentResult = surfaceAreaHeuristic(currentPlaneValue, dim, boundingBox,
                                                                 leftSize, middleSize, rightSize);

                if (Double::less(currentResult.first, minCost)) {
                    minCost = currentResult.first;
                    boundingBoxPlaneSplit.value = currentPlaneValue;
                    boundingBoxPlaneSplit.axis = dim;
                    boundingBoxPlaneSplit.type = currentResult.second;
                    boundingBoxPlaneSplit.cost = minCost;
                }
                leftSize += p_begin + p_belong;
            }
        }
        return boundingBoxPlaneSplit;
    }


    pnode recBuild(const vector<IGeometryObject *> &objects, const BoundingBox &boundingBox, size_t depth = 0) {
        pnode node = new KD_tree_node();
        node->boundingBox = boundingBox;

        const auto &split = findPlane(objects, boundingBox);
        if (Double::greater(split.cost, objects.size() * intersectionCoef) || depth == 7) {
            node->isLeaf = new KD_tree_leaf();
            node->isLeaf->objects = objects;
            //TODO do something
            return node;
        }

        const auto &boxes = boundingBox.split(split.axis, split.value);

        const auto &parts = classify(objects, boxes[0], boxes[1], split);

        node->splitPlane = make_pair(split.value, split.axis);
        node->nodeSize = objects.size();

        node->left = recBuild(parts[0], boxes[0], depth + 1);
        node->right = recBuild(parts[1], boxes[1], depth + 1);

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

        if (!node->boundingBox.intersect(ray).first) {
            return Intersection();
        }

        if (node->isLeaf != nullptr) {
            Intersection intersection;
            ldb intersectionCoef = 1e30;

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

/*
        pair<bool, ldb> planeCoef = getPlaneSplitCoef(node, ray);

        if (!planeCoef.first) {

            Intersection left = findIntersection(node->left, ray, coefs);
            Intersection right = findIntersection(node->right, ray, coefs);;
            if (left) {
                if (right && Double::greater(left.rayIntersectionCoef, right.rayIntersectionCoef) &&
                    Double::greater(right.rayIntersectionCoef, 0)) {
                    return right;
                }
                return left;
            }
            return right;
        }

        if (Double::less(coefs[1], planeCoef.second)) {
            Point outPoint = ray.begin + ray.direction * coefs[1];
            pnode part = node->left->boundingBox.contains(outPoint) ? node->left : node->right;
            return findIntersection(part, ray, coefs);

        } else if (Double::less(planeCoef.second, coefs[0])) {

            Point inPoint = ray.begin + ray.direction * coefs[0];
            pnode part = node->left->boundingBox.contains(inPoint) ? node->left : node->right;
            return findIntersection(part, ray, coefs);
        }
*/
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

    pnode root;

public:

    KD_Tree() {
        root = nullptr;
    }

    KD_Tree(vector<IGeometryObject *>
            objects) {
        root = nullptr;
        buildTree(objects);
    }

    void buildTree(vector<IGeometryObject *> objects) {
        if (root != nullptr) {
            delete root;
        }
        root = recBuild(objects, getBoundingBox(objects));
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

};


