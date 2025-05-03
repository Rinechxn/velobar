// include/menuitemmodel.hpp
#pragma once

#include <QObject>
#include <QVariantList>

class MenuItemModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList items READ items NOTIFY dataChanged)

public:
    explicit MenuItemModel(QObject* parent = nullptr);

    QVariantList items() const;
    void setItems(const QVariantList& allItems);

signals:
    void dataChanged();

private:
    QVariantList m_mainItems;
};
