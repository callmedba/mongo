/**
 *    Copyright (C) 2016 MongoDB Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Affero General Public License for more details.
 *
 *    You should have received a copy of the GNU Affero General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *    As a special exception, the copyright holders give permission to link the
 *    code of portions of this program with the OpenSSL library under certain
 *    conditions as described in each individual source file and distribute
 *    linked combinations including the program with the OpenSSL library. You
 *    must comply with the GNU Affero General Public License in all respects for
 *    all of the code used other than as permitted herein. If you modify file(s)
 *    with this exception, you may extend this exception to your version of the
 *    file(s), but you are not obligated to do so. If you do not wish to do so,
 *    delete this exception statement from your version. If you delete this
 *    exception statement from all source files in the program, then also delete
 *    it in the license file.
 */

#pragma once

#include <boost/optional.hpp>
#include <vector>

#include "mongo/bson/bsonelement.h"
#include "mongo/bson/bsonobj.h"
#include "mongo/db/namespace_string.h"
#include "mongo/db/query/explain_options.h"

namespace mongo {

template <typename T>
class StatusWith;
class Document;

/**
 * Represents the user-supplied options to the aggregate command.
 */
class AggregationRequest {
public:
    static const StringData kCommandName;
    static const StringData kCursorName;
    static const StringData kBatchSizeName;
    static const StringData kFromRouterName;
    static const StringData kPipelineName;
    static const StringData kCollationName;
    static const StringData kExplainName;
    static const StringData kAllowDiskUseName;
    static const StringData kHintName;

    static const long long kDefaultBatchSize;

    /**
     * Create a new instance of AggregationRequest by parsing the raw command object. Returns a
     * non-OK status if a required field was missing, if there was an unrecognized field name or if
     * there was a bad value for one of the fields.
     *
     * If we are parsing a request for an explained aggregation with an explain verbosity provided,
     * then 'explainVerbosity' contains this information. In this case, 'cmdObj' may not itself
     * contain the explain specifier. Otherwise, 'explainVerbosity' should be boost::none.
     */
    static StatusWith<AggregationRequest> parseFromBSON(
        NamespaceString nss,
        const BSONObj& cmdObj,
        boost::optional<ExplainOptions::Verbosity> explainVerbosity = boost::none);

    /**
     * Constructs an AggregationRequest over the given namespace with the given pipeline. All
     * options aside from the pipeline assume their default values.
     */
    AggregationRequest(NamespaceString nss, std::vector<BSONObj> pipeline);

    /**
     * Serializes the options to a Document. Note that this serialization includes the original
     * pipeline object, as specified. Callers will likely want to override this field with a
     * serialization of a parsed and optimized Pipeline object.
     *
     * The explain option is not serialized. Since the explain command format is {explain:
     * {aggregate: ...}, ...}, explain options are not part of the aggregate command object.
     */
    Document serializeToCommandObj() const;

    //
    // Getters.
    //

    long long getBatchSize() const {
        return _batchSize;
    }

    const NamespaceString& getNamespaceString() const {
        return _nss;
    }

    /**
     * An unparsed version of the pipeline. All BSONObjs are owned.
     */
    const std::vector<BSONObj>& getPipeline() const {
        return _pipeline;
    }

    bool isFromRouter() const {
        return _fromRouter;
    }

    bool shouldAllowDiskUse() const {
        return _allowDiskUse;
    }

    bool shouldBypassDocumentValidation() const {
        return _bypassDocumentValidation;
    }

    /**
     * Returns an empty object if no collation was specified.
     */
    BSONObj getCollation() const {
        return _collation;
    }

    BSONObj getHint() const {
        return _hint;
    }

    boost::optional<ExplainOptions::Verbosity> getExplain() const {
        return _explainMode;
    }

    //
    // Setters for optional fields.
    //

    /**
     * Negative batchSize is illegal but batchSize of 0 is allowed.
     */
    void setBatchSize(long long batchSize) {
        uassert(40203, "batchSize must be non-negative", batchSize >= 0);
        _batchSize = batchSize;
    }

    void setCollation(BSONObj collation) {
        _collation = collation.getOwned();
    }

    void setHint(BSONObj hint) {
        _hint = hint.getOwned();
    }

    void setExplain(boost::optional<ExplainOptions::Verbosity> verbosity) {
        _explainMode = verbosity;
    }

    void setAllowDiskUse(bool allowDiskUse) {
        _allowDiskUse = allowDiskUse;
    }

    void setFromRouter(bool isFromRouter) {
        _fromRouter = isFromRouter;
    }

    void setBypassDocumentValidation(bool shouldBypassDocumentValidation) {
        _bypassDocumentValidation = shouldBypassDocumentValidation;
    }

private:
    // Required fields.
    const NamespaceString _nss;

    // An unparsed version of the pipeline.
    const std::vector<BSONObj> _pipeline;

    long long _batchSize;

    // Optional fields.

    // An owned copy of the user-specified collation object, or an empty object if no collation was
    // specified.
    BSONObj _collation;

    // The hint provided, if any.  If the hint was by index key pattern, the value of '_hint' is
    // the key pattern hinted.  If the hint was by index name, the value of '_hint' is
    // {$hint: <String>}, where <String> is the index name hinted.
    BSONObj _hint;

    // The explain mode to use, or boost::none if this is not a request for an aggregation explain.
    boost::optional<ExplainOptions::Verbosity> _explainMode;

    bool _allowDiskUse = false;
    bool _fromRouter = false;
    bool _bypassDocumentValidation = false;
};
}  // namespace mongo
